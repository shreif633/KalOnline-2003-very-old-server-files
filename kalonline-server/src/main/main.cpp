#include "common/logger/Logger.hpp"
#include "common/config/Config.hpp"
#include "common/network/Network.hpp"
#include "main/GameServer.hpp"
#include <iostream>
#include <csignal>
#include <memory>

std::unique_ptr<asio::io_context> g_ioContext;
std::unique_ptr<kal::main::GameServer> g_gameServer;

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
        auto logger = kal::logger::Logger::getInstance();
        logger->add_backend(std::make_unique<kal::logger::ConsoleBackend>());
        logger->add_backend(std::make_unique<kal::logger::FileBackend>("logs/game_server.log"));
        
        KAL_LOG_INFO("KalOnline Game Server starting...");

        auto config = kal::config::Config::getInstance();
        std::string configPath = (argc > 1) ? argv[1] : "config.yaml";
        
        if (!config->load(configPath)) {
            KAL_LOG_ERROR("Failed to load configuration from {}", configPath);
            return 1;
        }

        g_ioContext = std::make_unique<asio::io_context>();
        uint16_t port = static_cast<uint16_t>(config->get<int>("game.port", 9003));
        
        g_gameServer = std::make_unique<kal::main::GameServer>(*g_ioContext, port);
        
        if (!g_gameServer->initialize()) {
            KAL_LOG_ERROR("Failed to initialize Game Server");
            return 1;
        }

        KAL_LOG_INFO("Game Server initialized on port {}. Starting service...", port);
        
        g_ioContext->run();

        KAL_LOG_INFO("Game Server shut down gracefully.");
        return 0;
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Fatal error: {}", e.what());
        return 1;
    }
}
