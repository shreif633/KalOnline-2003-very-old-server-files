#include "db/DBServer.hpp"
#include <format>
#include <bit>

namespace kal::db {

using namespace logger;

DBServer::DBServer(asio::io_context& io_context, uint16_t port,
                   std::shared_ptr<DatabasePool> db_pool)
    : TcpServer(io_context, port)
    , m_db_pool(std::move(db_pool))
{
    m_stats.start_time = std::chrono::system_clock::now();
    register_handlers();
    LOG_INFO("DBServer initialized on port {}", port);
}

DBServer::~DBServer() {
    stop();
}

void DBServer::start() {
    if (m_running.exchange(true)) {
        LOG_WARN("DBServer already running");
        return;
    }
    
    LOG_INFO("Starting DBServer...");
    TcpServer::start();
    LOG_INFO("DBServer started successfully");
}

void DBServer::stop() {
    if (!m_running.exchange(false)) {
        return;
    }
    
    LOG_INFO("Stopping DBServer...");
    TcpServer::stop();
    LOG_INFO("DBServer stopped");
}

void DBServer::register_handlers() {
    std::unique_lock lock(m_handlers_mutex);
    
    // Character operations
    m_handlers[QueryOpcode::LoadCharacter] = [this](auto&&... args) { 
        handle_load_character(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::SaveCharacter] = [this](auto&&... args) { 
        handle_save_character(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::CreateCharacter] = [this](auto&&... args) { 
        handle_create_character(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::DeleteCharacter] = [this](auto&&... args) { 
        handle_delete_character(std::forward<decltype(args)>(args)...); 
    };
    
    // Inventory operations
    m_handlers[QueryOpcode::LoadInventory] = [this](auto&&... args) { 
        handle_load_inventory(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::SaveInventory] = [this](auto&&... args) { 
        handle_save_inventory(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::MoveItem] = [this](auto&&... args) { 
        handle_move_item(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::DropItem] = [this](auto&&... args) { 
        handle_drop_item(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::EquipItem] = [this](auto&&... args) { 
        handle_equip_item(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::UnequipItem] = [this](auto&&... args) { 
        handle_unequip_item(std::forward<decltype(args)>(args)...); 
    };
    
    // Skill operations
    m_handlers[QueryOpcode::LoadSkills] = [this](auto&&... args) { 
        handle_load_skills(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::LearnSkill] = [this](auto&&... args) { 
        handle_learn_skill(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::UnlearnSkill] = [this](auto&&... args) { 
        handle_unlearn_skill(std::forward<decltype(args)>(args)...); 
    };
    
    // Quest operations
    m_handlers[QueryOpcode::LoadQuests] = [this](auto&&... args) { 
        handle_load_quests(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::UpdateQuest] = [this](auto&&... args) { 
        handle_update_quest(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::CompleteQuest] = [this](auto&&... args) { 
        handle_complete_quest(std::forward<decltype(args)>(args)...); 
    };
    
    // Social operations
    m_handlers[QueryOpcode::LoadFriends] = [this](auto&&... args) { 
        handle_load_friends(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::AddFriend] = [this](auto&&... args) { 
        handle_add_friend(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::RemoveFriend] = [this](auto&&... args) { 
        handle_remove_friend(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::LoadGuild] = [this](auto&&... args) { 
        handle_load_guild(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::CreateGuild] = [this](auto&&... args) { 
        handle_create_guild(std::forward<decltype(args)>(args)...); 
    };
    
    // Mail operations
    m_handlers[QueryOpcode::LoadMail] = [this](auto&&... args) { 
        handle_load_mail(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::SendMail] = [this](auto&&... args) { 
        handle_send_mail(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::DeleteMail] = [this](auto&&... args) { 
        handle_delete_mail(std::forward<decltype(args)>(args)...); 
    };
    
    // Economy operations
    m_handlers[QueryOpcode::BuyItem] = [this](auto&&... args) { 
        handle_buy_item(std::forward<decltype(args)>(args)...); 
    };
    m_handlers[QueryOpcode::SellItem] = [this](auto&&... args) { 
        handle_sell_item(std::forward<decltype(args)>(args)...); 
    };
    
    LOG_INFO("Registered {} query handlers", m_handlers.size());
}

void DBServer::on_packet_received(ConnectionPtr conn, const Packet& packet) {
    if (packet.size() < sizeof(uint16_t) * 2) {
        LOG_WARN("Invalid packet from {}: too small", conn->get_endpoint());
        conn->close();
        return;
    }
    
    // Parse opcode (first 2 bytes after length)
    const auto* data = packet.data();
    auto opcode = std::bit_cast<QueryOpcode>(data[sizeof(uint16_t)]);
    
    auto start_time = std::chrono::steady_clock::now();
    
    LOG_DEBUG("Received DB query: opcode={:#06x}, size={}", 
              static_cast<uint16_t>(opcode), packet.size());
    
    // Find and execute handler
    {
        std::shared_lock lock(m_handlers_mutex);
        auto it = m_handlers.find(opcode);
        if (it != m_handlers.end()) {
            try {
                it->second(conn, packet);
                
                auto end_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    end_time - start_time).count();
                
                log_query_performance(opcode, duration, true);
                m_stats.total_queries.fetch_add(1, std::memory_order_relaxed);
            } catch (const std::exception& e) {
                LOG_ERROR("Query handler exception: {}", e.what());
                auto response = create_response(opcode, QueryResult::DatabaseError);
                conn->send(response);
                
                m_stats.failed_queries.fetch_add(1, std::memory_order_relaxed);
            }
        } else {
            LOG_WARN("Unknown query opcode: {:#06x}", static_cast<uint16_t>(opcode));
            auto response = create_response(opcode, QueryResult::InvalidData);
            conn->send(response);
        }
    }
}

Packet DBServer::create_response(QueryOpcode opcode, QueryResult result, 
                                  std::span<const uint8_t> payload) {
    // Response format: [Length:2][Opcode:2][Result:1][Payload:N]
    const size_t header_size = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t);
    const size_t total_size = header_size + payload.size();
    
    Packet response(total_size);
    auto* data = response.data();
    
    // Length (excluding itself)
    auto length = static_cast<uint16_t>(total_size - sizeof(uint16_t));
    std::memcpy(data, &length, sizeof(uint16_t));
    
    // Opcode
    auto opcode_val = static_cast<uint16_t>(opcode);
    std::memcpy(data + sizeof(uint16_t), &opcode_val, sizeof(uint16_t));
    
    // Result code
    data[sizeof(uint16_t) * 2] = static_cast<uint8_t>(result);
    
    // Payload
    if (!payload.empty()) {
        std::memcpy(data + header_size, payload.data(), payload.size());
    }
    
    return response;
}

void DBServer::log_query_performance(QueryOpcode opcode, uint64_t duration_us, bool success) {
    if (!success) {
        LOG_WARN("Query failed: opcode={:#06x}, duration={}us", 
                 static_cast<uint16_t>(opcode), duration_us);
        return;
    }
    
    // Update moving average
    auto current_avg = m_stats.avg_response_time_us.load(std::memory_order_relaxed);
    auto total = m_stats.total_queries.load(std::memory_order_relaxed);
    if (total > 0) {
        auto new_avg = ((current_avg * total) + duration_us) / (total + 1);
        m_stats.avg_response_time_us.store(new_avg, std::memory_order_relaxed);
    }
    
    // Log slow queries (>10ms)
    if (duration_us > 10000) {
        LOG_WARN("Slow query detected: opcode={:#06x}, duration={}us", 
                 static_cast<uint16_t>(opcode), duration_us);
    }
}

// ============================================================================
// Character Query Handlers
// ============================================================================

void DBServer::handle_character_query(ConnectionPtr conn, const Packet& packet) {
    if (packet.size() < sizeof(uint16_t) * 2) {
        LOG_WARN("Invalid character query packet");
        return;
    }
    
    const auto* data = packet.data();
    auto sub_opcode = std::bit_cast<QueryOpcode>(data[sizeof(uint16_t)]);
    
    switch (sub_opcode) {
        case QueryOpcode::LoadCharacter:
            handle_load_character(conn, packet);
            break;
        case QueryOpcode::SaveCharacter:
            handle_save_character(conn, packet);
            break;
        case QueryOpcode::CreateCharacter:
            handle_create_character(conn, packet);
            break;
        case QueryOpcode::DeleteCharacter:
            handle_delete_character(conn, packet);
            break;
        default:
            LOG_WARN("Unknown character sub-opcode");
            break;
    }
}

void DBServer::handle_load_character(ConnectionPtr conn, const Packet& packet) {
    // Expected format: [Length:2][Opcode:2][CharacterID:4]
    if (packet.size() < sizeof(uint16_t) * 2 + sizeof(uint32_t)) {
        LOG_WARN("Invalid load character packet size");
        auto response = create_response(QueryOpcode::LoadCharacter, 
                                       QueryResult::InvalidData);
        conn->send(response);
        return;
    }
    
    const auto* data = packet.data();
    uint32_t character_id;
    std::memcpy(&character_id, data + sizeof(uint16_t) * 2, sizeof(uint32_t));
    
    LOG_DEBUG("Loading character ID: {}", character_id);
    
    try {
        auto db = m_db_pool->get_connection();
        
        // Load character data
        auto result = db->exec_params(
            "SELECT name, level, exp, str, hth, int, wis, dex, "
            "map_id, x, y, z, direction, job_class, faction, gold, "
            "bank_gold, guild_id, guild_rank, party_id, quest_progress "
            "FROM players WHERE character_id = $1",
            character_id
        );
        
        if (result.empty()) {
            LOG_WARN("Character not found: {}", character_id);
            auto response = create_response(QueryOpcode::LoadCharacter, 
                                           QueryResult::NotFound);
            conn->send(response);
            return;
        }
        
        const auto& row = result[0];
        
        // Build response payload
        std::vector<uint8_t> payload;
        payload.reserve(512);
        
        // Serialize character data
        // Format: [Name:32][Stats:...][Position:...][etc]
        std::string name = row["name"].as<std::string>();
        payload.resize(32, 0);
        std::memcpy(payload.data(), name.c_str(), std::min(name.size(), size_t(31)));
        
        // Add stats
        auto append_uint16 = [&payload](uint16_t val) {
            size_t pos = payload.size();
            payload.resize(pos + sizeof(uint16_t));
            std::memcpy(payload.data() + pos, &val, sizeof(uint16_t));
        };
        
        auto append_uint32 = [&payload](uint32_t val) {
            size_t pos = payload.size();
            payload.resize(pos + sizeof(uint32_t));
            std::memcpy(payload.data() + pos, &val, sizeof(uint32_t));
        };
        
        append_uint16(row["level"].as<uint16_t>());
        append_uint32(row["exp"].as<uint32_t>());
        append_uint16(row["str"].as<uint16_t>());
        append_uint16(row["hth"].as<uint16_t>());
        append_uint16(row["int"].as<uint16_t>());
        append_uint16(row["wis"].as<uint16_t>());
        append_uint16(row["dex"].as<uint16_t>());
        
        // Position
        append_uint16(row["map_id"].as<uint16_t>());
        float x = row["x"].as<float>();
        float y = row["y"].as<float>();
        float z = row["z"].as<float>();
        size_t pos = payload.size();
        payload.resize(pos + sizeof(float) * 3);
        std::memcpy(payload.data() + pos, &x, sizeof(float));
        std::memcpy(payload.data() + pos + sizeof(float), &y, sizeof(float));
        std::memcpy(payload.data() + pos + sizeof(float) * 2, &z, sizeof(float));
        
        append_uint16(row["direction"].as<uint16_t>());
        append_uint8(row["job_class"].as<uint8_t>());
        append_uint8(row["faction"].as<uint8_t>());
        
        // Gold
        uint64_t gold = row["gold"].as<uint64_t>();
        pos = payload.size();
        payload.resize(pos + sizeof(uint64_t));
        std::memcpy(payload.data() + pos, &gold, sizeof(uint64_t));
        
        uint64_t bank_gold = row["bank_gold"].as<uint64_t>();
        pos = payload.size();
        payload.resize(pos + sizeof(uint64_t));
        std::memcpy(payload.data() + pos, &bank_gold, sizeof(uint64_t));
        
        append_uint32(row["guild_id"].is_null() ? 0 : row["guild_id"].as<uint32_t>());
        append_uint8(row["guild_rank"].is_null() ? 0 : row["guild_rank"].as<uint8_t>());
        append_uint32(row["party_id"].is_null() ? 0 : row["party_id"].as<uint32_t>());
        append_uint32(row["quest_progress"].is_null() ? 0 : row["quest_progress"].as<uint32_t>());
        
        auto response = create_response(QueryOpcode::LoadCharacter, 
                                       QueryResult::Success, payload);
        conn->send(response);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load character {}: {}", character_id, e.what());
        auto response = create_response(QueryOpcode::LoadCharacter, 
                                       QueryResult::DatabaseError);
        conn->send(response);
    }
}

void DBServer::handle_save_character(ConnectionPtr conn, const Packet& packet) {
    // Implementation for saving character data
    LOG_DEBUG("Save character request received");
    // TODO: Implement character save logic
    auto response = create_response(QueryOpcode::SaveCharacter, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_create_character(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Create character request received");
    // TODO: Implement character creation
    auto response = create_response(QueryOpcode::CreateCharacter, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_delete_character(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Delete character request received");
    // TODO: Implement character deletion
    auto response = create_response(QueryOpcode::DeleteCharacter, QueryResult::Success);
    conn->send(response);
}

// ============================================================================
// Inventory Query Handlers
// ============================================================================

void DBServer::handle_inventory_query(ConnectionPtr conn, const Packet& packet) {
    // Route to specific inventory handler based on sub-opcode
    handle_load_inventory(conn, packet);
}

void DBServer::handle_load_inventory(ConnectionPtr conn, const Packet& packet) {
    if (packet.size() < sizeof(uint16_t) * 2 + sizeof(uint32_t)) {
        LOG_WARN("Invalid load inventory packet");
        auto response = create_response(QueryOpcode::LoadInventory, 
                                       QueryResult::InvalidData);
        conn->send(response);
        return;
    }
    
    const auto* data = packet.data();
    uint32_t character_id;
    std::memcpy(&character_id, data + sizeof(uint16_t) * 2, sizeof(uint32_t));
    
    LOG_DEBUG("Loading inventory for character {}", character_id);
    
    try {
        auto db = m_db_pool->get_connection();
        
        auto result = db->exec_params(
            "SELECT item_blob FROM player_inventory WHERE character_id = $1",
            character_id
        );
        
        if (result.empty()) {
            // No inventory yet, return empty
            auto response = create_response(QueryOpcode::LoadInventory, 
                                           QueryResult::Success);
            conn->send(response);
            return;
        }
        
        const auto& row = result[0];
        auto blob = row["item_blob"].as<std::vector<uint8_t>>();
        
        auto response = create_response(QueryOpcode::LoadInventory, 
                                       QueryResult::Success, blob);
        conn->send(response);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load inventory: {}", e.what());
        auto response = create_response(QueryOpcode::LoadInventory, 
                                       QueryResult::DatabaseError);
        conn->send(response);
    }
}

void DBServer::handle_save_inventory(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Save inventory request received");
    // TODO: Implement inventory save with blob serialization
    auto response = create_response(QueryOpcode::SaveInventory, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_move_item(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Move item request received");
    auto response = create_response(QueryOpcode::MoveItem, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_drop_item(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Drop item request received");
    auto response = create_response(QueryOpcode::DropItem, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_equip_item(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Equip item request received");
    auto response = create_response(QueryOpcode::EquipItem, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_unequip_item(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Unequip item request received");
    auto response = create_response(QueryOpcode::UnequipItem, QueryResult::Success);
    conn->send(response);
}

// ============================================================================
// Skill Query Handlers
// ============================================================================

void DBServer::handle_skill_query(ConnectionPtr conn, const Packet& packet) {
    handle_load_skills(conn, packet);
}

void DBServer::handle_load_skills(ConnectionPtr conn, const Packet& packet) {
    if (packet.size() < sizeof(uint16_t) * 2 + sizeof(uint32_t)) {
        LOG_WARN("Invalid load skills packet");
        auto response = create_response(QueryOpcode::LoadSkills, 
                                       QueryResult::InvalidData);
        conn->send(response);
        return;
    }
    
    const auto* data = packet.data();
    uint32_t character_id;
    std::memcpy(&character_id, data + sizeof(uint16_t) * 2, sizeof(uint32_t));
    
    LOG_DEBUG("Loading skills for character {}", character_id);
    
    try {
        auto db = m_db_pool->get_connection();
        
        auto result = db->exec_params(
            "SELECT skill_blob FROM player_skills WHERE character_id = $1",
            character_id
        );
        
        if (result.empty()) {
            auto response = create_response(QueryOpcode::LoadSkills, 
                                           QueryResult::Success);
            conn->send(response);
            return;
        }
        
        const auto& row = result[0];
        auto blob = row["skill_blob"].as<std::vector<uint8_t>>();
        
        auto response = create_response(QueryOpcode::LoadSkills, 
                                       QueryResult::Success, blob);
        conn->send(response);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load skills: {}", e.what());
        auto response = create_response(QueryOpcode::LoadSkills, 
                                       QueryResult::DatabaseError);
        conn->send(response);
    }
}

void DBServer::handle_learn_skill(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Learn skill request received");
    auto response = create_response(QueryOpcode::LearnSkill, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_unlearn_skill(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Unlearn skill request received");
    auto response = create_response(QueryOpcode::UnlearnSkill, QueryResult::Success);
    conn->send(response);
}

// ============================================================================
// Quest Query Handlers
// ============================================================================

void DBServer::handle_quest_query(ConnectionPtr conn, const Packet& packet) {
    handle_load_quests(conn, packet);
}

void DBServer::handle_load_quests(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Load quests request received");
    auto response = create_response(QueryOpcode::LoadQuests, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_update_quest(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Update quest request received");
    auto response = create_response(QueryOpcode::UpdateQuest, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_complete_quest(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Complete quest request received");
    auto response = create_response(QueryOpcode::CompleteQuest, QueryResult::Success);
    conn->send(response);
}

// ============================================================================
// Social Query Handlers
// ============================================================================

void DBServer::handle_social_query(ConnectionPtr conn, const Packet& packet) {
    handle_load_friends(conn, packet);
}

void DBServer::handle_load_friends(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Load friends request received");
    auto response = create_response(QueryOpcode::LoadFriends, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_add_friend(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Add friend request received");
    auto response = create_response(QueryOpcode::AddFriend, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_remove_friend(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Remove friend request received");
    auto response = create_response(QueryOpcode::RemoveFriend, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_load_guild(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Load guild request received");
    auto response = create_response(QueryOpcode::LoadGuild, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_create_guild(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Create guild request received");
    auto response = create_response(QueryOpcode::CreateGuild, QueryResult::Success);
    conn->send(response);
}

// ============================================================================
// Mail Query Handlers
// ============================================================================

void DBServer::handle_mail_query(ConnectionPtr conn, const Packet& packet) {
    handle_load_mail(conn, packet);
}

void DBServer::handle_load_mail(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Load mail request received");
    auto response = create_response(QueryOpcode::LoadMail, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_send_mail(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Send mail request received");
    auto response = create_response(QueryOpcode::SendMail, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_delete_mail(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Delete mail request received");
    auto response = create_response(QueryOpcode::DeleteMail, QueryResult::Success);
    conn->send(response);
}

// ============================================================================
// Economy Query Handlers
// ============================================================================

void DBServer::handle_economy_query(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Economy query received");
}

void DBServer::handle_buy_item(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Buy item request received");
    auto response = create_response(QueryOpcode::BuyItem, QueryResult::Success);
    conn->send(response);
}

void DBServer::handle_sell_item(ConnectionPtr conn, const Packet& packet) {
    LOG_DEBUG("Sell item request received");
    auto response = create_response(QueryOpcode::SellItem, QueryResult::Success);
    conn->send(response);
}

// ============================================================================
// Blob Serialization Helpers
// ============================================================================

std::vector<uint8_t> DBServer::serialize_inventory(const std::vector<Item>& items) {
    std::vector<uint8_t> blob;
    // TODO: Implement proper item serialization matching original format
    return blob;
}

std::expected<std::vector<Item>, std::string> 
DBServer::deserialize_inventory(std::span<const uint8_t> blob) {
    // TODO: Implement proper item deserialization matching original format
    return std::vector<Item>{};
}

std::vector<uint8_t> DBServer::serialize_skills(const std::vector<Skill>& skills) {
    std::vector<uint8_t> blob;
    // TODO: Implement proper skill serialization
    return blob;
}

std::expected<std::vector<Skill>, std::string> 
DBServer::deserialize_skills(std::span<const uint8_t> blob) {
    // TODO: Implement proper skill deserialization
    return std::vector<Skill>{};
}

} // namespace kal::db
