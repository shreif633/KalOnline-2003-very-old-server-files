#pragma once

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include "common/logger/Logger.hpp"

#define KAL_LOG_INFO(...) kal::logger::Logger::getInstance()->info(__VA_ARGS__)
#define KAL_LOG_ERROR(...) kal::logger::Logger::getInstance()->error(__VA_ARGS__)
#define KAL_LOG_WARNING(...) kal::logger::Logger::getInstance()->warning(__VA_ARGS__)

namespace kal::database {

class DbConnection {
public:
    explicit DbConnection(const std::string& connection_string);
    ~DbConnection();
    
    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;
    DbConnection(DbConnection&&) noexcept;
    DbConnection& operator=(DbConnection&&) noexcept;
    
    bool is_connected() const noexcept;
    void reconnect();
    pqxx::connection& get_connection() { return *m_conn; }
    const std::string& get_connection_string() const { return m_connection_string; }

private:
    std::unique_ptr<pqxx::connection> m_conn;
    std::string m_connection_string;
};

struct QueryResult {
    bool success = false;
    std::string error_message;
    size_t rows_affected = 0;
    
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> column_names;
    
    bool next() { return current_row < rows.size(); }
    void advance() { ++current_row; }
    
    std::string GetString(size_t col) const { 
        return current_row < rows.size() && col < rows[current_row].size() 
            ? rows[current_row][col] : ""; 
    }
    int GetInt(size_t col) const { 
        return current_row < rows.size() && col < rows[current_row].size() 
            ? std::stoi(rows[current_row][col]) : 0; 
    }
    float GetFloat(size_t col) const { 
        return current_row < rows.size() && col < rows[current_row].size() 
            ? std::stof(rows[current_row][col]) : 0.0f; 
    }
    bool IsNull(size_t col) const {
        return current_row >= rows.size() || col >= rows[current_row].size() || rows[current_row][col].empty();
    }
    
private:
    size_t current_row = 0;
};

class Database {
public:
    static Database* GetInstance();
    static void DestroyInstance();
    
    bool Initialize(const std::string& connection_string, size_t pool_size = 5);
    void Shutdown();
    
    std::unique_ptr<QueryResult> ExecuteQuery(const std::string& query, const std::vector<std::string>& params = {});
    bool ExecuteNonQuery(const std::string& query, const std::vector<std::string>& params = {});
    
    // Prepared statements
    bool PrepareStatement(const std::string& name, const std::string& query);
    std::unique_ptr<QueryResult> ExecutePrepared(const std::string& name, const std::vector<std::string>& params = {});

private:
    Database() = default;
    ~Database();
    
    std::unique_ptr<DbConnection> getConnection();
    void returnConnection(std::unique_ptr<DbConnection> conn);
    
    static Database* instance;
    std::string m_connection_string;
    std::vector<std::unique_ptr<DbConnection>> m_pool;
    std::queue<std::unique_ptr<DbConnection>> m_available;
    std::mutex m_pool_mutex;
    std::condition_variable m_pool_cv;
    
    std::unordered_map<std::string, std::string> m_prepared_statements;
};

} // namespace kal::database
