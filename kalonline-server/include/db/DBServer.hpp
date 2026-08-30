#pragma once

#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include "common/logger/Logger.hpp"
#include <asio.hpp>
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <functional>
#include <expected>
#include <span>

namespace kal::db {

using namespace network;
using namespace database;

// Query opcodes (original 2003 protocol)
enum class QueryOpcode : uint16_t {
    // Character operations
    LoadCharacter = 0x0101,
    SaveCharacter = 0x0102,
    CreateCharacter = 0x0103,
    DeleteCharacter = 0x0104,
    
    // Inventory operations
    LoadInventory = 0x0201,
    SaveInventory = 0x0202,
    MoveItem = 0x0203,
    DropItem = 0x0204,
    EquipItem = 0x0205,
    UnequipItem = 0x0206,
    
    // Skill operations
    LoadSkills = 0x0301,
    LearnSkill = 0x0302,
    UnlearnSkill = 0x0303,
    
    // Quest operations
    LoadQuests = 0x0401,
    UpdateQuest = 0x0402,
    CompleteQuest = 0x0403,
    
    // Social operations
    LoadFriends = 0x0501,
    AddFriend = 0x0502,
    RemoveFriend = 0x0503,
    LoadGuild = 0x0504,
    CreateGuild = 0x0505,
    
    // Mail operations
    LoadMail = 0x0601,
    SendMail = 0x0602,
    DeleteMail = 0x0603,
    
    // Shop/Economy
    BuyItem = 0x0701,
    SellItem = 0x0702,
    
    // Response codes
    Success = 0x0000,
    Error = 0xFFFF
};

// Query result codes
enum class QueryResult : uint8_t {
    Success = 0,
    NotFound = 1,
    DatabaseError = 2,
    InvalidData = 3,
    PermissionDenied = 4,
    AlreadyExists = 5
};

// Statistics
struct DBStats {
    std::atomic<uint64_t> total_queries{0};
    std::atomic<uint64_t> failed_queries{0};
    std::atomic<uint64_t> avg_response_time_us{0};
    std::chrono::system_clock::time_point start_time;
    
    DBStats() : start_time(std::chrono::system_clock::now()) {}
};

// Query handler function type
using QueryHandler = std::function<void(ConnectionPtr, const Packet&)>;

class DBServer : public TcpServer {
public:
    explicit DBServer(asio::io_context& io_context, uint16_t port,
                     std::shared_ptr<DatabasePool> db_pool);
    
    ~DBServer();
    
    void start() override;
    void stop() override;
    
    void on_packet_received(ConnectionPtr conn, const Packet& packet) override;
    
    // Query handlers by category
    void handle_character_query(ConnectionPtr conn, const Packet& packet);
    void handle_inventory_query(ConnectionPtr conn, const Packet& packet);
    void handle_skill_query(ConnectionPtr conn, const Packet& packet);
    void handle_quest_query(ConnectionPtr conn, const Packet& packet);
    void handle_social_query(ConnectionPtr conn, const Packet& packet);
    void handle_mail_query(ConnectionPtr conn, const Packet& packet);
    void handle_economy_query(ConnectionPtr conn, const Packet& packet);
    
    // Specific query handlers
    void handle_load_character(ConnectionPtr conn, const Packet& packet);
    void handle_save_character(ConnectionPtr conn, const Packet& packet);
    void handle_create_character(ConnectionPtr conn, const Packet& packet);
    void handle_delete_character(ConnectionPtr conn, const Packet& packet);
    
    void handle_load_inventory(ConnectionPtr conn, const Packet& packet);
    void handle_save_inventory(ConnectionPtr conn, const Packet& packet);
    void handle_move_item(ConnectionPtr conn, const Packet& packet);
    void handle_drop_item(ConnectionPtr conn, const Packet& packet);
    void handle_equip_item(ConnectionPtr conn, const Packet& packet);
    void handle_unequip_item(ConnectionPtr conn, const Packet& packet);
    
    void handle_load_skills(ConnectionPtr conn, const Packet& packet);
    void handle_learn_skill(ConnectionPtr conn, const Packet& packet);
    void handle_unlearn_skill(ConnectionPtr conn, const Packet& packet);
    
    void handle_load_quests(ConnectionPtr conn, const Packet& packet);
    void handle_update_quest(ConnectionPtr conn, const Packet& packet);
    void handle_complete_quest(ConnectionPtr conn, const Packet& packet);
    
    void handle_load_friends(ConnectionPtr conn, const Packet& packet);
    void handle_add_friend(ConnectionPtr conn, const Packet& packet);
    void handle_remove_friend(ConnectionPtr conn, const Packet& packet);
    
    void handle_load_guild(ConnectionPtr conn, const Packet& packet);
    void handle_create_guild(ConnectionPtr conn, const Packet& packet);
    
    void handle_load_mail(ConnectionPtr conn, const Packet& packet);
    void handle_send_mail(ConnectionPtr conn, const Packet& packet);
    void handle_delete_mail(ConnectionPtr conn, const Packet& packet);
    
    void handle_buy_item(ConnectionPtr conn, const Packet& packet);
    void handle_sell_item(ConnectionPtr conn, const Packet& packet);
    
    const DBStats& get_stats() const { return m_stats; }
    
private:
    void register_handlers();
    Packet create_response(QueryOpcode opcode, QueryResult result, 
                          std::span<const uint8_t> payload = {});
    void log_query_performance(QueryOpcode opcode, uint64_t duration_us, bool success);
    
    // Blob serialization helpers
    static std::vector<uint8_t> serialize_inventory(const std::vector<Item>& items);
    static std::expected<std::vector<Item>, std::string> deserialize_inventory(
        std::span<const uint8_t> blob);
    
    static std::vector<uint8_t> serialize_skills(const std::vector<Skill>& skills);
    static std::expected<std::vector<Skill>, std::string> deserialize_skills(
        std::span<const uint8_t> blob);
    
    std::shared_ptr<DatabasePool> m_db_pool;
    std::unordered_map<QueryOpcode, QueryHandler> m_handlers;
    mutable std::shared_mutex m_handlers_mutex;
    DBStats m_stats;
    std::atomic<bool> m_running{false};
};

} // namespace kal::db
