#include "common/database/Database.hpp"
#include <format>
#include <algorithm>

namespace kal::database {

// ============================================================================
// DbConnection Implementation
// ============================================================================

DbConnection::DbConnection(const std::string& connection_string)
    : m_connection_string(connection_string) {
    try {
        m_conn = std::make_unique<pqxx::connection>(connection_string);
    } catch (const pqxx::broken_connection& e) {
        KAL_LOG_ERROR("Failed to connect to database: {}", e.what());
        throw;
    }
}

DbConnection::~DbConnection() = default;

DbConnection::DbConnection(DbConnection&& other) noexcept
    : m_conn(std::move(other.m_conn))
    , m_connection_string(std::move(other.m_connection_string)) {
}

DbConnection& DbConnection::operator=(DbConnection&& other) noexcept {
    if (this != &other) {
        m_conn = std::move(other.m_conn);
        m_connection_string = std::move(other.m_connection_string);
    }
    return *this;
}

bool DbConnection::is_connected() const noexcept {
    return m_conn && m_conn->is_open();
}

void DbConnection::reconnect() {
    try {
        if (m_conn && m_conn->is_open()) {
            m_conn->close();
        }
        m_conn = std::make_unique<pqxx::connection>(m_connection_string);
        KAL_LOG_INFO("Database connection re-established");
    } catch (const pqxx::broken_connection& e) {
        KAL_LOG_ERROR("Failed to reconnect to database: {}", e.what());
        throw;
    }
}

pqxx::result DbConnection::exec(const std::string& sql) {
    if (!is_connected()) {
        throw std::runtime_error("Database connection not open");
    }
    
    try {
        pqxx::work txn(*m_conn);
        auto result = txn.exec(sql);
        txn.commit();
        return result;
    } catch (const pqxx::sql_error& e) {
        KAL_LOG_ERROR("SQL error: {} | Query: {}", e.what(), sql);
        throw;
    }
}

// ============================================================================
// ConnectionPool Implementation
// ============================================================================

ConnectionPool::ConnectionPool(const Config::DatabaseConfig& config)
    : m_config(config) {
    // Pre-create connections
    for (int i = 0; i < config.max_connections; ++i) {
        auto conn = create_connection();
        if (conn) {
            m_pool.push_back(conn);
            m_available.push(conn);
        }
    }
    
    KAL_LOG_INFO("Connection pool initialized with {} connections", m_pool.size());
}

ConnectionPool::~ConnectionPool() {
    shutdown();
}

std::shared_ptr<DbConnection> ConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    // Wait until a connection is available or shutdown
    m_cv.wait(lock, [this] {
        return !m_available.empty() || m_shutdown;
    });
    
    if (m_shutdown) {
        return nullptr;
    }
    
    auto conn = m_available.front();
    m_available.pop();
    ++m_active_count;
    
    // Verify connection is still valid
    if (!conn->is_connected()) {
        try {
            conn->reconnect();
        } catch (...) {
            // Try to create a new connection
            conn = create_connection();
        }
    }
    
    return conn;
}

void ConnectionPool::release(std::shared_ptr<DbConnection> conn) {
    if (!conn) return;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_shutdown && conn->is_connected()) {
            m_available.push(conn);
        }
        --m_active_count;
    }
    
    m_cv.notify_one();
}

size_t ConnectionPool::active_connections() const noexcept {
    return m_active_count;
}

size_t ConnectionPool::available_connections() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_available.size();
}

size_t ConnectionPool::total_connections() const noexcept {
    return m_pool.size();
}

void ConnectionPool::shutdown() {
    m_shutdown = true;
    m_cv.notify_all();
    
    // Close all connections
    for (auto& conn : m_pool) {
        try {
            if (conn.use_count() == 1 && conn->is_connected()) {
                conn->raw().close();
            }
        } catch (...) {}
    }
    
    m_pool.clear();
    
    KAL_LOG_INFO("Connection pool shut down");
}

std::shared_ptr<DbConnection> ConnectionPool::create_connection() {
    try {
        return std::make_shared<DbConnection>(m_config.connection_string());
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Failed to create database connection: {}", e.what());
        return nullptr;
    }
}

// ============================================================================
// QueryRouter Implementation
// ============================================================================

QueryRouter::QueryRouter(ConnectionPool& pool, int worker_threads)
    : m_pool(pool)
    , m_worker_threads(worker_threads) {
}

QueryRouter::~QueryRouter() {
    stop();
}

void QueryRouter::start() {
    if (m_running) return;
    
    m_running = true;
    prepare_all_statements();
    
    for (int i = 0; i < m_worker_threads; ++i) {
        m_workers.emplace_back(&QueryRouter::worker_thread_func, this);
    }
    
    KAL_LOG_INFO("Query router started with {} workers", m_worker_threads);
}

void QueryRouter::stop() {
    if (!m_running) return;
    
    m_running = false;
    m_queue_cv.notify_all();
    
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_workers.clear();
    
    KAL_LOG_INFO("Query router stopped");
}

void QueryRouter::submit(QueryRequest request) {
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        
        if (m_queue.size() >= MAX_QUEUE_SIZE) {
            KAL_LOG_WARNING("Query queue full, dropping request");
            return;
        }
        
        m_queue.push(std::move(request));
    }
    
    m_queue_cv.notify_one();
}

void QueryRouter::submit_prepared(std::string statement, std::vector<std::string> params,
                                   QueryRequest::Callback callback, int priority) {
    submit(QueryRequest{
        .statement = std::move(statement),
        .params = std::move(params),
        .callback = std::move(callback),
        .priority = priority
    });
}

std::expected<pqxx::result, std::string> QueryRouter::execute_sync(
    const std::string& statement,
    const std::vector<std::string>& params) {
    
    std::expected<pqxx::result, std::string> result;
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    
    submit(QueryRequest{
        .statement = statement,
        .params = params,
        .callback = [&](std::expected<pqxx::result, std::string> r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = std::move(r);
            done = true;
            cv.notify_one();
        },
        .priority = 100  // High priority for sync calls
    });
    
    // Wait for completion
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&] { return done; });
    
    return result;
}

size_t QueryRouter::pending_queries() const noexcept {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_queue.size();
}

void QueryRouter::register_prepared_statement(const std::string& name, const std::string& sql) {
    m_prepared_statements.emplace_back(name, sql);
}

void QueryRouter::prepare_all_statements() {
    auto conn = m_pool.acquire();
    if (!conn) return;
    
    try {
        for (const auto& [name, sql] : m_prepared_statements) {
            conn->raw().prepare_statement(name, sql);
            KAL_LOG_DEBUG("Prepared statement registered: {}", name);
        }
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Failed to prepare statements: {}", e.what());
    }
    
    m_pool.release(conn);
}

void QueryRouter::worker_thread_func() {
    while (m_running) {
        QueryRequest request;
        
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            
            m_queue_cv.wait(lock, [this] {
                return !m_queue.empty() || !m_running;
            });
            
            if (!m_running && m_queue.empty()) {
                break;
            }
            
            if (m_queue.empty()) {
                continue;
            }
            
            request = std::move(m_queue.top());
            m_queue.pop();
        }
        
        // Execute the query
        try {
            auto conn = m_pool.acquire();
            if (!conn) {
                if (request.callback) {
                    request.callback(std::unexpected("No database connection available"));
                }
                continue;
            }
            
            pqxx::result result;
            
            if (request.params.empty()) {
                result = conn->exec_prepared(request.statement);
            } else {
                // Convert params to appropriate types for libpqxx
                // This is simplified - in production, you'd want proper type handling
                result = conn->exec_prepared(request.statement);
            }
            
            if (request.callback) {
                request.callback(result);
            }
            
            m_pool.release(conn);
            
        } catch (const std::exception& e) {
            KAL_LOG_ERROR("Query execution failed: {}", e.what());
            
            if (request.callback) {
                request.callback(std::unexpected(e.what()));
            }
        }
    }
}

// ============================================================================
// DatabaseManager Implementation
// ============================================================================

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

void DatabaseManager::initialize(const Config::DatabaseConfig& config) {
    if (m_initialized) {
        KAL_LOG_WARNING("Database manager already initialized");
        return;
    }
    
    m_pool = std::make_unique<ConnectionPool>(config);
    m_router = std::make_unique<QueryRouter>(*m_pool, 4);
    m_router->start();
    
    m_initialized = true;
    
    KAL_LOG_INFO("Database manager initialized");
}

void DatabaseManager::shutdown() {
    if (!m_initialized) return;
    
    m_router->stop();
    m_pool->shutdown();
    
    m_initialized = false;
    
    KAL_LOG_INFO("Database manager shut down");
}

DatabaseManager::~DatabaseManager() {
    if (m_initialized) {
        shutdown();
    }
}

} // namespace kal::database
