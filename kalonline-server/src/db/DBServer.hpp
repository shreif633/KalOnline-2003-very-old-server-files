#pragma once
#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include "db/core/QueryRouter.hpp"
#include <asio.hpp>
#include <memory>
#include <atomic>
#include <chrono>

namespace kal::db {

struct DBStats {
    std::chrono::system_clock::time_point start_time;
    uint64_t totalQueries;
    uint64_t failedQueries;
    uint64_t activeConnections;
};

class DBServer : public network::TcpServer {
public:
    DBServer(asio::io_context& io_context, uint16_t port,
             std::shared_ptr<database::Database> db);
    ~DBServer();

    void Start();
    void Stop();
    
    DBStats GetStats() const;

protected:
    void OnMessage(network::ConnectionPtr conn, 
                   std::span<const uint8_t> data) override;
    void OnConnect(network::ConnectionPtr conn) override;
    void OnDisconnect(network::ConnectionPtr conn) override;

private:
    void RegisterHandlers();
    
    // Query handlers
    void HandleCharacterLoad(network::ConnectionPtr conn, 
                            std::span<const uint8_t> packet);
    void HandleCharacterSave(network::ConnectionPtr conn, 
                            std::span<const uint8_t> packet);
    void HandleInventoryLoad(network::ConnectionPtr conn, 
                            std::span<const uint8_t> packet);
    void HandleInventorySave(network::ConnectionPtr conn, 
                            std::span<const uint8_t> packet);
    void HandleSkillLoad(network::ConnectionPtr conn, 
                        std::span<const uint8_t> packet);
    void HandleGuildLoad(network::ConnectionPtr conn, 
                        std::span<const uint8_t> packet);
    
    std::shared_ptr<database::Database> m_db;
    std::unique_ptr<QueryRouter> m_query_router;
    
    mutable std::mutex m_stats_mutex;
    DBStats m_stats;
    
    std::atomic<bool> m_running{false};
};

} // namespace kal::db
