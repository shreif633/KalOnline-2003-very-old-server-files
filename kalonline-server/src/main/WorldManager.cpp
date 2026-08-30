#include "main/CombatSystem.hpp"
#include "common/logger/Logger.hpp"
#include <fstream>
#include <filesystem>

namespace kal::main {

using namespace logger;

// ============================================================================
// MapData Implementation
// ============================================================================

bool MapData::load_from_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open map file: {}", path);
        return false;
    }
    
    // Read map header
    file.read(reinterpret_cast<char*>(&header), sizeof(MapHeader));
    
    if (header.magic != 0x4B414C4D) { // "KALM"
        LOG_ERROR("Invalid map file format: {}", path);
        return false;
    }
    
    LOG_INFO("Loading map {} ({}x{})", header.name, header.width, header.height);
    
    // Load terrain data
    terrain.resize(header.width * header.height);
    file.read(reinterpret_cast<char*>(terrain.data()), 
              terrain.size() * sizeof(TerrainTile));
    
    // Load collision data
    collision.resize(header.width * header.height);
    file.read(reinterpret_cast<char*>(collision.data()),
              collision.size() * sizeof(CollisionBlock));
    
    // Load spawn points
    uint32_t spawn_count;
    file.read(reinterpret_cast<char*>(&spawn_count), sizeof(uint32_t));
    
    spawn_points.resize(spawn_count);
    file.read(reinterpret_cast<char*>(spawn_points.data()),
              spawn_count * sizeof(SpawnPoint));
    
    LOG_INFO("Map loaded: {} spawn points", spawn_count);
    return true;
}

auto MapData::get_terrain(float x, float y) const -> TerrainType {
    if (x < 0 || x >= header.width || y < 0 || y >= header.height) {
        return TerrainType::Invalid;
    }
    
    size_t index = static_cast<size_t>(y) * header.width + 
                   static_cast<size_t>(x);
    return terrain[index].type;
}

bool MapData::is_walkable(float x, float y) const {
    if (x < 0 || x >= header.width || y < 0 || y >= header.height) {
        return false;
    }
    
    size_t index = static_cast<size_t>(y) * header.width + 
                   static_cast<size_t>(x);
    return !collision[index].is_blocked;
}

float MapData::get_terrain_height(float x, float y) const {
    if (x < 0 || x >= header.width || y < 0 || y >= header.height) {
        return 0.0f;
    }
    
    size_t index = static_cast<size_t>(y) * header.width + 
                   static_cast<size_t>(x);
    return static_cast<float>(terrain[index].height);
}

// ============================================================================
// WorldManager Implementation
// ============================================================================

WorldManager::WorldManager(std::shared_ptr<DatabasePool> db_pool)
    : m_db_pool(std::move(db_pool))
{
    LOG_INFO("WorldManager initialized");
}

WorldManager::~WorldManager() {
    stop();
}

bool WorldManager::initialize(const std::string& maps_directory) {
    LOG_INFO("Initializing WorldManager with maps from: {}", maps_directory);
    
    namespace fs = std::filesystem;
    
    if (!fs::exists(maps_directory)) {
        LOG_ERROR("Maps directory does not exist: {}", maps_directory);
        return false;
    }
    
    // Load all map files
    for (const auto& entry : fs::directory_iterator(maps_directory)) {
        if (entry.path().extension() == ".nbi") {
            MapData map;
            if (map.load_from_file(entry.path().string())) {
                m_maps[map.header.map_id] = std::move(map);
                LOG_INFO("Loaded map: {} (ID: {})", 
                        map.header.name, map.header.map_id);
            }
        }
    }
    
    if (m_maps.empty()) {
        LOG_WARN("No maps loaded!");
        return false;
    }
    
    LOG_INFO("WorldManager initialized with {} maps", m_maps.size());
    return true;
}

void WorldManager::start() {
    if (m_running.exchange(true)) {
        return;
    }
    
    LOG_INFO("Starting WorldManager...");
    m_world_thread = std::thread([this]() { world_tick_loop(); });
    m_running = true;
}

void WorldManager::stop() {
    if (!m_running.exchange(false)) {
        return;
    }
    
    LOG_INFO("Stopping WorldManager...");
    
    if (m_world_thread.joinable()) {
        m_world_thread.join();
    }
    
    // Save all player positions
    save_all_players();
}

auto WorldManager::get_map(uint16_t map_id) const -> const MapData* {
    auto it = m_maps.find(map_id);
    if (it != m_maps.end()) {
        return &it->second;
    }
    return nullptr;
}

bool WorldManager::is_valid_position(uint16_t map_id, float x, float y, float z) const {
    auto map = get_map(map_id);
    if (!map) {
        return false;
    }
    
    // Check bounds
    if (x < 0 || x >= map->header.width || 
        y < 0 || y >= map->header.height) {
        return false;
    }
    
    // Check collision
    if (!map->is_walkable(x, y)) {
        return false;
    }
    
    // Check Z coordinate (should match terrain height)
    float terrain_z = map->get_terrain_height(x, y);
    if (std::abs(z - terrain_z) > 10.0f) {
        return false; // Too high or low
    }
    
    return true;
}

void WorldManager::world_tick_loop() {
    LOG_INFO("World tick loop started (interval={}ms)", WORLD_TICK_MS);
    
    auto last_tick = std::chrono::steady_clock::now();
    
    while (m_running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_tick).count();
        
        if (elapsed >= WORLD_TICK_MS) {
            uint32_t current_time = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count());
            
            process_effects(current_time);
            check_respawn_monsters();
            cleanup_expired_drops(current_time);
            
            last_tick = now;
        }
        
        // Small sleep to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    LOG_INFO("World tick loop stopped");
}

void WorldManager::process_effects(uint32_t current_time) {
    std::shared_lock lock(m_entities_mutex);
    
    for (auto& [id, entity] : m_entities) {
        if (auto* character = dynamic_cast<Character*>(entity.get())) {
            // Process DoTs and buffs
            std::erase_if(character->active_effects, [current_time](const Effect& effect) {
                if (current_time >= effect.end_time) {
                    LOG_DEBUG("Effect {} expired", effect.effect_id);
                    return true;
                }
                
                // Tick damage/heal
                if (effect.tick_interval > 0 && 
                    current_time - effect.last_tick >= effect.tick_interval * 1000) {
                    
                    if (effect.is_debuff) {
                        character->take_damage(effect.param_value, effect.source_id);
                    } else {
                        character->heal(effect.param_value);
                    }
                    
                    effect.last_tick = current_time;
                }
                
                return false;
            });
        }
    }
}

void WorldManager::check_respawn_monsters() {
    uint32_t current_time = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    
    std::unique_lock lock(m_entities_mutex);
    
    for (auto& [id, entity] : m_entities) {
        if (auto* monster = dynamic_cast<Monster*>(entity.get())) {
            if (monster->is_dead() && current_time >= monster->death_time + monster->respawn_time) {
                // Respawn at spawn point
                monster->x = monster->spawn_x;
                monster->y = monster->spawn_y;
                monster->z = monster->spawn_z;
                monster->stats.current_hp = monster->stats.max_hp;
                monster->stats.current_mp = monster->stats.max_mp;
                monster->damage_dealt.clear();
                
                LOG_DEBUG("Monster {} respawned at ({}, {}, {})", 
                         monster->monster_id, monster->x, monster->y, monster->z);
                
                // Broadcast respawn packet
                Packet respawn_packet(20);
                auto* ptr = reinterpret_cast<uint32_t*>(respawn_packet.data());
                ptr[0] = 0x0301; // OpCode for monster spawn
                ptr[1] = monster->id;
                ptr[2] = monster->monster_id;
                ptr[3] = static_cast<uint32_t>(monster->x * 1000);
                ptr[4] = static_cast<uint32_t>(monster->y * 1000);
                
                broadcast_to_nearby(monster->id, respawn_packet);
            }
        }
    }
}

void WorldManager::cleanup_expired_drops(uint32_t current_time) {
    std::unique_lock lock(m_drops_mutex);
    
    std::erase_if(m_ground_items, [current_time](const auto& pair) {
        bool expired = current_time >= pair.second.expire_time;
        if (expired) {
            LOG_DEBUG("Drop item {} expired", pair.first);
        }
        return expired;
    });
}

void WorldManager::save_all_players() {
    LOG_INFO("Saving all player data...");
    
    std::shared_lock lock(m_entities_mutex);
    
    int saved_count = 0;
    for (auto& [id, entity] : m_entities) {
        if (auto* player = dynamic_cast<Player*>(entity.get())) {
            // Would call MainServer::save_player here
            LOG_DEBUG("Player {} saved at ({}, {}, {})", 
                     player->name, player->x, player->y, player->z);
            saved_count++;
        }
    }
    
    LOG_INFO("Saved {} players", saved_count);
}

void WorldManager::broadcast_to_nearby(uint32_t entity_id, const Packet& packet) {
    std::shared_lock lock(m_entities_mutex);
    
    auto it = m_entities.find(entity_id);
    if (it == m_entities.end()) {
        return;
    }
    
    const auto& entity = it->second;
    float radius = 150.0f; // Visibility radius
    
    for (auto& [id, other] : m_entities) {
        if (id == entity_id) continue;
        
        float dx = entity->x - other->x;
        float dy = entity->y - other->y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= radius && entity->map_id == other->map_id) {
            if (auto* other_player = dynamic_cast<Player*>(other.get())) {
                if (other_player->connection) {
                    other_player->connection->send(packet);
                }
            }
        }
    }
}

void WorldManager::send_to_player(uint32_t entity_id, const Packet& packet) {
    std::shared_lock lock(m_entities_mutex);
    
    auto it = m_entities.find(entity_id);
    if (it != m_entities.end()) {
        if (auto* player = dynamic_cast<Player*>(it->second.get())) {
            if (player->connection) {
                player->connection->send(packet);
            }
        }
    }
}

auto WorldManager::get_nearby_characters(uint32_t entity_id, float radius) const 
    -> std::vector<std::shared_ptr<Character>> {
    std::shared_lock lock(m_entities_mutex);
    
    std::vector<std::shared_ptr<Character>> result;
    
    auto it = m_entities.find(entity_id);
    if (it == m_entities.end()) {
        return result;
    }
    
    const auto& entity = it->second;
    
    for (auto& [id, other] : m_entities) {
        if (id == entity_id) continue;
        
        float dx = entity->x - other->x;
        float dy = entity->y - other->y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= radius && entity->map_id == other->map_id) {
            if (auto* character = dynamic_cast<Character*>(other.get())) {
                result.push_back(std::static_pointer_cast<Character>(other));
            }
        }
    }
    
    return result;
}

} // namespace kal::main
