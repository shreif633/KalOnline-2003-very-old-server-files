#pragma once
#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include "db/core/QueryRouter.hpp"
#include <asio.hpp>
#include <memory>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <bit>
#include <cstring>
#include <expected>

namespace kal::db {

// Forward declarations
struct Item {
    uint32_t id{0};
    uint32_t type_id{0};
    uint32_t quantity{1};
    uint8_t slot{0};
    uint8_t grade{0};
    uint16_t durability{0};
};

struct Skill {
    uint32_t id{0};
    uint8_t level{1};
};

enum class ServerQueryOpcode : uint16_t {
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
    
    // Economy operations
    BuyItem = 0x0701,
    SellItem = 0x0702
};

enum class ServerQueryResult : uint8_t {
    Success = 0,
    DatabaseError = 1,
    NotFound = 2,
    InvalidData = 3
};

using Packet = std::vector<uint8_t>;
using ConnectionPtr = std::shared_ptr<network::Connection>;

struct DBStats {
    std::chrono::system_clock::time_point start_time;
    std::atomic<uint64_t> total_queries{0};
    std::atomic<uint64_t> failed_queries{0};
    std::atomic<uint64_t> active_connections{0};
    std::atomic<uint64_t> avg_response_time_us{0};
};

class DatabasePool;

class DBServer {
public:
    DBServer(asio::io_context& io_context, uint16_t port,
             std::shared_ptr<DatabasePool> db_pool);
    ~DBServer();

    void start();
    void stop();
    
    const DBStats& GetStats() const { return m_stats; }

private:
    using HandlerFunc = std::function<void(ConnectionPtr, const Packet&)>;
    
    void register_handlers();
    void on_packet_received(ConnectionPtr conn, const Packet& packet);
    Packet create_response(ServerQueryOpcode opcode, ServerQueryResult result,
                          std::span<const uint8_t> payload = {});
    void log_query_performance(ServerQueryOpcode opcode, uint64_t duration_us, bool success);
    
    // Character handlers
    void handle_character_query(ConnectionPtr conn, const Packet& packet);
    void handle_load_character(ConnectionPtr conn, const Packet& packet);
    void handle_save_character(ConnectionPtr conn, const Packet& packet);
    void handle_create_character(ConnectionPtr conn, const Packet& packet);
    void handle_delete_character(ConnectionPtr conn, const Packet& packet);
    
    // Inventory handlers
    void handle_inventory_query(ConnectionPtr conn, const Packet& packet);
    void handle_load_inventory(ConnectionPtr conn, const Packet& packet);
    void handle_save_inventory(ConnectionPtr conn, const Packet& packet);
    void handle_move_item(ConnectionPtr conn, const Packet& packet);
    void handle_drop_item(ConnectionPtr conn, const Packet& packet);
    void handle_equip_item(ConnectionPtr conn, const Packet& packet);
    void handle_unequip_item(ConnectionPtr conn, const Packet& packet);
    
    // Skill handlers
    void handle_skill_query(ConnectionPtr conn, const Packet& packet);
    void handle_load_skills(ConnectionPtr conn, const Packet& packet);
    void handle_learn_skill(ConnectionPtr conn, const Packet& packet);
    void handle_unlearn_skill(ConnectionPtr conn, const Packet& packet);
    
    // Quest handlers
    void handle_quest_query(ConnectionPtr conn, const Packet& packet);
    void handle_load_quests(ConnectionPtr conn, const Packet& packet);
    void handle_update_quest(ConnectionPtr conn, const Packet& packet);
    void handle_complete_quest(ConnectionPtr conn, const Packet& packet);
    
    // Social handlers
    void handle_social_query(ConnectionPtr conn, const Packet& packet);
    void handle_load_friends(ConnectionPtr conn, const Packet& packet);
    void handle_add_friend(ConnectionPtr conn, const Packet& packet);
    void handle_remove_friend(ConnectionPtr conn, const Packet& packet);
    void handle_load_guild(ConnectionPtr conn, const Packet& packet);
    void handle_create_guild(ConnectionPtr conn, const Packet& packet);
    
    // Mail handlers
    void handle_mail_query(ConnectionPtr conn, const Packet& packet);
    void handle_load_mail(ConnectionPtr conn, const Packet& packet);
    void handle_send_mail(ConnectionPtr conn, const Packet& packet);
    void handle_delete_mail(ConnectionPtr conn, const Packet& packet);
    
    // Economy handlers
    void handle_economy_query(ConnectionPtr conn, const Packet& packet);
    void handle_buy_item(ConnectionPtr conn, const Packet& packet);
    void handle_sell_item(ConnectionPtr conn, const Packet& packet);
    
    // Serialization helpers
    std::vector<uint8_t> serialize_inventory(const std::vector<Item>& items);
    std::expected<std::vector<Item>, std::string> deserialize_inventory(std::span<const uint8_t> blob);
    std::vector<uint8_t> serialize_skills(const std::vector<Skill>& skills);
    std::expected<std::vector<Skill>, std::string> deserialize_skills(std::span<const uint8_t> blob);
    
    std::shared_ptr<DatabasePool> m_db_pool;
    std::unique_ptr<network::TcpServer> m_server;
    
    mutable std::mutex m_stats_mutex;
    DBStats m_stats;
    
    std::unordered_map<ServerQueryOpcode, HandlerFunc> m_handlers;
    mutable std::shared_mutex m_handlers_mutex;
    
    std::atomic<bool> m_running{false};
};

} // namespace kal::db
