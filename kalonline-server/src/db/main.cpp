#include "common/logger/Logger.hpp"
#include "common/config/Config.hpp"
#include "common/network/Network.hpp"
#include "DBServer.hpp"
#include <iostream>
#include <csignal>
#include <memory>

std::unique_ptr<asio::io_context> g_ioContext;
std::unique_ptr<kal::db::DbServer> g_dbServer;

void signalHandler(int signum) {
    KAL_LOG_INFO("Interrupt signal ({}) received. Shutting down...", signum);
    if (g_ioContext) {
        g_ioContext->stop();
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        auto& logger = kal::logger::Logger::getInstance();
        logger.add_backend(std::make_unique<kal::logger::ConsoleBackend>());
        logger.add_backend(std::make_unique<kal::logger::FileBackend>("logs/db_server.log"));
        
        KAL_LOG_INFO("KalOnline DB Server starting...");

        auto& config = kal::config::Config::getInstance();
        std::string configPath = (argc > 1) ? argv[1] : "config.yaml";
        
        if (!config.load(configPath)) {
            KAL_LOG_ERROR("Failed to load configuration from {}", configPath);
            return 1;
        }

        g_ioContext = std::make_unique<asio::io_context>();
        uint16_t port = static_cast<uint16_t>(config.get<int>("db.port", 9002));
        
        g_dbServer = std::make_unique<kal::db::DbServer>(*g_ioContext, port);
        
        KAL_LOG_INFO("DB Server initialized on port {}. Starting service...", port);
        
        // Start the server
        g_dbServer->Start();
        
        g_ioContext->run();

        // Stop server
        if (g_dbServer) {
            g_dbServer->Stop();
        }

        KAL_LOG_INFO("DB Server shut down gracefully.");
        return 0;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Fatal error: {}", e.what());
        return 1;
    }
}
