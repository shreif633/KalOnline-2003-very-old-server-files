#include <iostream>
#include <csignal>
#include <atomic>

#include "common/logger/Logger.hpp"
#include "common/config/Config.hpp"
#include "common/database/Database.hpp"
#include "common/network/Network.hpp"

using namespace kal;

static std::atomic<bool> g_running{true};

void signal_handler(int signum) {
    KAL_LOG_INFO("Received signal {}, shutting down...", signum);
    g_running = false;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    logger::AsyncLogger::instance().initialize("KalDBServer", logger::LogLevel::Info);
    
    KAL_LOG_INFO("========================================");
    KAL_LOG_INFO("KalOnline Database Server (C++23 Modern)");
    KAL_LOG_INFO("========================================");
    
    std::string config_path = "config/config.yaml";
    if (argc > 1) {
        config_path = argv[1];
    }
    
    if (!config::Config::instance().load(config_path)) {
        KAL_LOG_ERROR("Failed to load configuration from {}", config_path);
        return EXIT_FAILURE;
    }
    
    KAL_LOG_INFO("Configuration loaded from {}", config_path);
    
    try {
        auto db_config = config::Config::instance().database();
        database::DatabaseManager::instance().initialize(db_config);
        
        asio::io_context io_context;
        network::ConnectionManager conn_manager;
        
        uint16_t port = config::Config::instance().get_int("network.db_port", 11001);
        network::TcpServer server(io_context, port);
        server.set_connection_manager(conn_manager);
        
        server.start();
        
        KAL_LOG_INFO("DB server started on port {}", port);
        KAL_LOG_INFO("Press Ctrl+C to stop...");
        
        while (g_running) {
            try {
                io_context.run_for(std::chrono::milliseconds(100));
            } catch (const std::exception& e) {
                KAL_LOG_ERROR("IO context error: {}", e.what());
            }
        }
        
        KAL_LOG_INFO("Shutting down DB server...");
        server.stop();
        conn_manager.stop_all();
        database::DatabaseManager::instance().shutdown();
        logger::AsyncLogger::instance().shutdown();
        
        KAL_LOG_INFO("DB server stopped gracefully");
        
    } catch (const std::exception& e) {
        KAL_LOG_FATAL("Fatal error: {}", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
