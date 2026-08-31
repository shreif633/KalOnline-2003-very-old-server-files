#pragma once
#include <pqxx/pqxx>
#include <memory>
#include <vector>
#include <mutex>

namespace kal::db {

class DatabasePool {
public:
    static DatabasePool& instance() {
        static DatabasePool pool;
        return pool;
    }
    
    std::shared_ptr<pqxx::connection> get_connection() {
        std::lock_guard<std::mutex> lock(mutex_);
        // Return a dummy connection for now
        return std::make_shared<pqxx::connection>("dbname=kalonline user=postgres password=postgres host=localhost");
    }
    
private:
    DatabasePool() = default;
    std::mutex mutex_;
};

} // namespace kal::db
