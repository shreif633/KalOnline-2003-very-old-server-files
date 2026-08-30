#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include "common/logger/Logger.hpp"

namespace kal::database {

struct QueryResult {
    bool success;
    std::vector<std::vector<std::string>> rows;
    std::string error;
    
    bool Next() const { return !rows.empty(); }
    std::string GetString(size_t col) const { return rows.empty() ? "" : rows[0][col]; }
    int GetInt(size_t col) const { return rows.empty() ? 0 : std::stoi(rows[0][col]); }
    float GetFloat(size_t col) const { return rows.empty() ? 0.0f : std::stof(rows[0][col]); }
};

class Database {
public:
    static Database* getInstance();
    
    bool connect(const std::string& host, const std::string& port, 
                 const std::string& dbname, const std::string& user, 
                 const std::string& password);
    void disconnect();
    bool isConnected() const;
    
    std::unique_ptr<QueryResult> executeQuery(const std::string& query, 
                                               const std::vector<std::string>& params = {});
    bool executeCommand(const std::string& query, 
                       const std::vector<std::string>& params = {});
    
    // Transaction support
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    Database() = default;
    ~Database();
    
    std::unique_ptr<pqxx::connection> m_connection;
    std::unique_ptr<pqxx::work> m_transaction;
    static Database* s_instance;
};

} // namespace kal::database
