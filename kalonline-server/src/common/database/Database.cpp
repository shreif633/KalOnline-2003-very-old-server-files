#include "common/database/Database.hpp"

namespace kal::database {

Database* Database::s_instance = nullptr;

Database* Database::getInstance() {
    if (!s_instance) {
        s_instance = new Database();
    }
    return s_instance;
}

Database::~Database() {
    disconnect();
}

bool Database::connect(const std::string& host, const std::string& port,
                       const std::string& dbname, const std::string& user,
                       const std::string& password) {
    try {
        std::string connStr = "host=" + host + " port=" + port + 
                             " dbname=" + dbname + " user=" + user + 
                             " password=" + password;
        
        m_connection = std::make_unique<pqxx::connection>(connStr);
        
        if (m_connection->is_open()) {
            KAL_LOG_INFO("Database connected successfully to {}:{}", host, port);
            return true;
        }
        
        KAL_LOG_ERROR("Failed to open database connection");
        return false;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Database connection failed: {}", e.what());
        return false;
    }
}

void Database::disconnect() {
    if (m_transaction) {
        rollbackTransaction();
    }
    
    if (m_connection) {
        m_connection->close();
        m_connection.reset();
        KAL_LOG_INFO("Database disconnected");
    }
}

bool Database::isConnected() const {
    return m_connection && m_connection->is_open();
}

std::unique_ptr<QueryResult> Database::executeQuery(const std::string& query,
                                                     const std::vector<std::string>& params) {
    auto result = std::make_unique<QueryResult>();
    
    if (!isConnected()) {
        result->success = false;
        result->error = "Not connected to database";
        return result;
    }
    
    try {
        pqxx::work txn(*m_connection);
        pqxx::result res;
        
        if (params.empty()) {
            res = txn.exec(query);
        } else {
            // Use prepared statement for parameters
            pqxx::stream_from stream(txn, query);
        }
        
        for (const auto& row : res) {
            std::vector<std::string> rowData;
            for (size_t i = 0; i < row.size(); ++i) {
                std::string value;
                row[i].to(value);
                rowData.push_back(value);
            }
            result->rows.push_back(rowData);
        }
        
        result->success = true;
        return result;
    } catch (const std::exception& e) {
        result->success = false;
        result->error = e.what();
        KAL_LOG_ERROR("Query execution failed: {}", e.what());
        return result;
    }
}

bool Database::executeCommand(const std::string& query,
                              const std::vector<std::string>& params) {
    if (!isConnected()) {
        return false;
    }
    
    try {
        pqxx::work txn(*m_connection);
        
        if (params.empty()) {
            txn.exec0(query);
        }
        
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Command execution failed: {}", e.what());
        return false;
    }
}

bool Database::beginTransaction() {
    if (!isConnected() || m_transaction) {
        return false;
    }
    
    try {
        m_transaction = std::make_unique<pqxx::work>(*m_connection);
        return true;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Begin transaction failed: {}", e.what());
        return false;
    }
}

bool Database::commitTransaction() {
    if (!m_transaction) {
        return false;
    }
    
    try {
        m_transaction->commit();
        m_transaction.reset();
        return true;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Commit transaction failed: {}", e.what());
        m_transaction.reset();
        return false;
    }
}

bool Database::rollbackTransaction() {
    if (!m_transaction) {
        return false;
    }
    
    try {
        m_transaction->abort();
        m_transaction.reset();
        return true;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Rollback transaction failed: {}", e.what());
        m_transaction.reset();
        return false;
    }
}

} // namespace kal::database
