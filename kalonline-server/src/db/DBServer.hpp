#pragma once
#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include "db/core/QueryRouter.hpp"
#include <asio.hpp>
#include <memory>
#include <atomic>
#include <chrono>

namespace kal::db {

using namespace kal::network;
using namespace kal::database;

struct DBStats {
    std::chrono::system_clock::time_point start_time;
    uint64_t totalQueries;
    uint64_t failedQueries;
    uint64_t activeConnections;
};

class DBServer : public TcpServer {
public:
    DBServer(asio::io_context& io_context, uint16_t port,
             std::shared_ptr<DatabasePool> db_pool);
    ~DBServer();

    void Start();
    void Stop();
    
    DBStats GetStats() const;

protected:
    void OnMessage(std::shared_ptr<TcpConnection> conn, 
                   std::span<const uint8_t> data) override;
    void OnConnect(std::shared_ptr<TcpConnection> conn) override;
    void OnDisconnect(std::shared_ptr<TcpConnection> conn) override;

private:
    void RegisterHandlers();
    
    // Query handlers
    void HandleCharacterLoad(std::shared_ptr<TcpConnection> conn, 
                            std::span<const uint8_t> packet);
    void HandleCharacterSave(std::shared_ptr<TcpConnection> conn, 
                            std::span<const uint8_t> packet);
    void HandleInventoryLoad(std::shared_ptr<TcpConnection> conn, 
                            std::span<const uint8_t> packet);
    void HandleInventorySave(std::shared_ptr<TcpConnection> conn, 
                            std::span<const uint8_t> packet);
    void HandleSkillLoad(std::shared_ptr<TcpConnection> conn, 
                        std::span<const uint8_t> packet);
    void HandleGuildLoad(std::shared_ptr<TcpConnection> conn, 
                        std::span<const uint8_t> packet);
    
    std::shared_ptr<DatabasePool> m_db_pool;
    std::unique_ptr<QueryRouter> m_query_router;
    
    mutable std::mutex m_stats_mutex;
    DBStats m_stats;
    
    std::atomic<bool> m_running{false};
};

} // namespace kal::db
