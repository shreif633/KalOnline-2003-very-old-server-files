#pragma once

#include "main/CombatSystem.hpp"
#include "common/database/Database.hpp"
#include <thread>
#include <shared_mutex>
#include <filesystem>

namespace kal::main {

// Map file structures (matching original KalOnline .nbi format)
#pragma pack(push, 1)

struct MapHeader {
    uint32_t magic;          // "KALM" = 0x4B414C4D
    uint16_t map_id;
    char name[32];
    uint16_t width;
    uint16_t height;
    uint16_t terrain_version;
    uint16_t collision_version;
    uint32_t flags;
};

struct TerrainTile {
    uint8_t type;            // TerrainType enum
    int16_t height;          // Height in meters * 100
    uint8_t walkable;        // 0 = blocked, 1 = walkable
    uint8_t reserved;
};

struct CollisionBlock {
    uint8_t is_blocked;
    uint8_t is_water;
    uint8_t is_safe_zone;
    uint8_t reserved;
};

struct SpawnPoint {
    uint32_t spawn_id;
    uint32_t monster_id;
    float x, y, z;
    uint16_t radius;
    uint16_t count;
    uint32_t respawn_time_ms;
};

#pragma pack(pop)

enum class TerrainType : uint8_t {
    Grass = 0,
    Dirt = 1,
    Stone = 2,
    Water = 3,
    Sand = 4,
    Snow = 5,
    Lava = 6,
    Invalid = 255
};

class MapData {
public:
    bool load_from_file(const std::string& path);
    
    auto get_terrain(float x, float y) const -> TerrainType;
    bool is_walkable(float x, float y) const;
    float get_terrain_height(float x, float y) const;
    
    const MapHeader& get_header() const { return header; }
    const auto& get_spawn_points() const { return spawn_points; }
    
private:
    MapHeader header{};
    std::vector<TerrainTile> terrain;
    std::vector<CollisionBlock> collision;
    std::vector<SpawnPoint> spawn_points;
};

class WorldManager {
public:
    explicit WorldManager(std::shared_ptr<database::DatabasePool> db_pool);
    ~WorldManager();
    
    bool initialize(const std::string& maps_directory);
    void start();
    void stop();
    
    // Map access
    auto get_map(uint16_t map_id) const -> const MapData*;
    bool is_valid_position(uint16_t map_id, float x, float y, float z) const;
    
    // Entity management (delegates to MainServer's entity containers)
    void set_entities(
        std::unordered_map<uint32_t, std::shared_ptr<GameObject>>* entities,
        std::unordered_map<uint32_t, std::shared_ptr<Player>>* players) {
        m_entities_ptr = entities;
        m_players_ptr = players;
    }
    
    // Accessors for entity containers (used by MainServer)
    auto get_entities() const -> const std::unordered_map<uint32_t, std::shared_ptr<GameObject>>* {
        return m_entities_ptr;
    }
    
    // World tick operations
    void process_effects(uint32_t current_time);
    void check_respawn_monsters();
    void cleanup_expired_drops(uint32_t current_time);
    
    // Broadcasting
    void broadcast_to_nearby(uint32_t entity_id, const network::Packet& packet);
    void send_to_player(uint32_t entity_id, const network::Packet& packet);
    auto get_nearby_characters(uint32_t entity_id, float radius) const 
        -> std::vector<std::shared_ptr<Character>>;
    
    // Drop management
    void set_ground_items(std::unordered_map<uint32_t, DropItem>* drops) {
        m_drops_ptr = drops;
    }
    
private:
    void world_tick_loop();
    void save_all_players();
    
    std::shared_ptr<database::DatabasePool> m_db_pool;
    std::unordered_map<uint16_t, MapData> m_maps;
    
    // Pointers to MainServer's entity containers
    std::unordered_map<uint32_t, std::shared_ptr<GameObject>>* m_entities_ptr{nullptr};
    std::unordered_map<uint32_t, std::shared_ptr<Player>>* m_players_ptr{nullptr};
    std::unordered_map<uint32_t, DropItem>* m_drops_ptr{nullptr};
    
    mutable std::shared_mutex m_entities_mutex;
    mutable std::mutex m_drops_mutex;
    
    std::thread m_world_thread;
    std::atomic<bool> m_running{false};
    
    static constexpr uint32_t WORLD_TICK_MS = 100;
};

} // namespace kal::main
