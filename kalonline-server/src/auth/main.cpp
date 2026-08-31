#include "common/logger/Logger.hpp"
#include "common/config/Config.hpp"
#include "common/network/Network.hpp"
#include "auth/AuthServer.hpp"
#include <iostream>
#include <csignal>
#include <memory>

std::unique_ptr<asio::io_context> g_ioContext;
std::unique_ptr<kal::auth::AuthServer> g_authServer;

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
        // Initialize logger first
        auto& logger = kal::logger::AsyncLogger::instance();
        logger.initialize("auth_server");
        
        KAL_LOG_INFO("KalOnline Auth Server starting...");

        // Load configuration
        std::string configPath = (argc > 1) ? argv[1] : "../config/config.yaml";
        
        auto& config = kal::config::Config::instance();
        if (!config.load(configPath)) {
            KAL_LOG_ERROR("Failed to load configuration from {}", configPath);
            return 1;
        }
        KAL_LOG_INFO("Configuration loaded from {}", configPath);

        // Create IO context and server
        g_ioContext = std::make_unique<asio::io_context>();
        uint16_t port = config.get_int("network.auth_port", 11000);
        
        g_authServer = std::make_unique<kal::auth::AuthServer>(*g_ioContext, port);
        
        KAL_LOG_INFO("Auth Server initialized on port {}. Starting service...", port);
        
        // Start the server
        g_authServer->Start();
        
        // Run the io context (this blocks until stop() is called)
        g_ioContext->run();

        // Stop server
        if (g_authServer) {
            g_authServer->Stop();
        }

        KAL_LOG_INFO("Auth Server shut down gracefully.");
        return 0;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Fatal error: {}", e.what());
        return 1;
    }
}
