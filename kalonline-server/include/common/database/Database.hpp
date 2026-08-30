#pragma once

#include <pqxx/pqxx>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <expected>
#include <functional>

#include "common/logger/Logger.hpp"
#include "common/config/Config.hpp"

namespace kal::database {

/**
 * @brief Database connection wrapper with prepared statement support
 */
class DbConnection {
public:
    explicit DbConnection(const std::string& connection_string);
    ~DbConnection();
    
    // Non-copyable, movable
    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;
    DbConnection(DbConnection&&) noexcept;
    DbConnection& operator=(DbConnection&&) noexcept;
    
    [[nodiscard]] bool is_connected() const noexcept;
    void reconnect();
    
    // Transaction helpers
    template<typename Func>
    auto transaction(Func&& func) -> decltype(func(std::declval<pqxx::work&>())) {
        pqxx::work txn(*m_conn);
        auto result = func(txn);
        txn.commit();
        return result;
    }
    
    // Prepared statement execution
    template<typename... Args>
    pqxx::result exec_prepared(const std::string& statement, Args&&... args) {
        return m_conn->exec_prepared(statement, std::forward<Args>(args)...);
    }
    
    // Direct SQL execution (use sparingly, prefer prepared statements)
    pqxx::result exec(const std::string& sql);
    
    [[nodiscard]] pqxx::connection& raw() noexcept { return *m_conn; }
    [[nodiscard]] const pqxx::connection& raw() const noexcept { return *m_conn; }
    
private:
    std::unique_ptr<pqxx::connection> m_conn;
    std::string m_connection_string;
};

/**
 * @brief Connection pool for efficient database access
 */
class ConnectionPool {
public:
    explicit ConnectionPool(const Config::DatabaseConfig& config);
    ~ConnectionPool();
    
    // Non-copyable
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    
    [[nodiscard]] std::shared_ptr<DbConnection> acquire();
    void release(std::shared_ptr<DbConnection> conn);
    
    [[nodiscard]] size_t active_connections() const noexcept;
    [[nodiscard]] size_t available_connections() const noexcept;
    [[nodiscard]] size_t total_connections() const noexcept;
    
    void shutdown();
    
private:
    std::shared_ptr<DbConnection> create_connection();
    
    Config::DatabaseConfig m_config;
    std::vector<std::shared_ptr<DbConnection>> m_pool;
    std::queue<std::shared_ptr<DbConnection>> m_available;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<size_t> m_active_count{0};
    std::atomic<bool> m_shutdown{false};
};

/**
 * @brief Async query request for queue-based processing
 */
struct QueryRequest {
    using Callback = std::function<void(std::expected<pqxx::result, std::string>)>;
    
    std::string statement;
    std::vector<std::string> params;
    Callback callback;
    int priority{0};  // Higher = more urgent
    
    bool operator<(const QueryRequest& other) const {
        return priority < other.priority;  // For max-heap
    }
};

/**
 * @brief Query router for distributing database operations
 * Supports sync and async query execution with priority queuing
 */
class QueryRouter {
public:
    explicit QueryRouter(ConnectionPool& pool, int worker_threads = 4);
    ~QueryRouter();
    
    // Start/stop worker threads
    void start();
    void stop();
    
    // Submit queries
    void submit(QueryRequest request);
    
    // Convenience methods
    void submit_prepared(std::string statement, std::vector<std::string> params,
                         QueryRequest::Callback callback, int priority = 0);
    
    // Sync execution (use sparingly)
    std::expected<pqxx::result, std::string> execute_sync(
        const std::string& statement, 
        const std::vector<std::string>& params = {});
    
    [[nodiscard]] size_t pending_queries() const noexcept;
    [[nodiscard]] bool is_running() const noexcept { return m_running; }
    
    // Prepared statement registration
    void register_prepared_statement(const std::string& name, const std::string& sql);
    void prepare_all_statements();
    
private:
    void worker_thread_func();
    
    ConnectionPool& m_pool;
    int m_worker_threads;
    std::atomic<bool> m_running{false};
    std::vector<std::jthread> m_workers;
    
    std::priority_queue<QueryRequest> m_queue;
    mutable std::mutex m_queue_mutex;
    std::condition_variable m_queue_cv;
    
    std::vector<std::pair<std::string, std::string>> m_prepared_statements;
    
    static constexpr int MAX_QUEUE_SIZE = 10000;
};

/**
 * @brief Database manager - singleton facade for all database operations
 */
class DatabaseManager {
public:
    static DatabaseManager& instance();
    
    void initialize(const Config::DatabaseConfig& config);
    void shutdown();
    
    [[nodiscard]] ConnectionPool& pool() noexcept { return *m_pool; }
    [[nodiscard]] QueryRouter& router() noexcept { return *m_router; }
    
    [[nodiscard]] bool is_initialized() const noexcept { return m_initialized; }
    
private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    std::unique_ptr<ConnectionPool> m_pool;
    std::unique_ptr<QueryRouter> m_router;
    std::atomic<bool> m_initialized{false};
};

// Helper macros for common queries
#define KAL_DB_EXEC_PREPARED(stmt, ...) \
    ::kal::database::DatabaseManager::instance().router().execute_sync(stmt, {__VA_ARGS__})

#define KAL_DB_SUBMIT_ASYNC(stmt, params, callback) \
    ::kal::database::DatabaseManager::instance().router().submit_prepared(stmt, params, callback)

} // namespace kal::database
