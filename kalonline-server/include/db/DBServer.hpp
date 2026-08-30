#pragma once

#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace kal::db {

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

class DBServer : public network::TcpServer {
public:
    explicit DBServer(asio::io_context& io_context, uint16_t port,
                     std::shared_ptr<database::DatabasePool> db_pool);
    
    ~DBServer();
    
    void start();
    void stop();
    
    void on_packet_received(network::ConnectionPtr conn, const network::Packet& packet);
    
    // Query handlers
    void handle_character_query(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_inventory_query(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_skill_query(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_quest_query(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_social_query(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_mail_query(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_economy_query(network::ConnectionPtr conn, const network::Packet& packet);
    
    const DBStats& get_stats() const { return m_stats; }
    
private:
    // Helper methods
    network::Packet create_response(QueryOpcode opcode, QueryResult result);
    void log_query_performance(QueryOpcode opcode, uint64_t duration_us, bool success);
    
    std::shared_ptr<database::DatabasePool> m_db_pool;
    DBStats m_stats;
    std::atomic<bool> m_running{false};
};

} // namespace kal::db
