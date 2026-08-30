#include "AuthServer.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "Database.hpp"
#include <csignal>
#include <atomic>

std::atomic<bool> g_running(true);

void SignalHandler(int signal) {
    Logger::Info("Received signal {}, shutting down...", signal);
    g_running = false;
}

int main(int argc, char* argv[]) {
    // Setup signal handlers
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
    
    try {
        // Initialize logging
        Logger::Init("kal-auth", LogLevel::Info);
        Logger::Info("KalOnline Auth Server starting...");
        
        // Load configuration
        auto configPath = (argc > 1) ? argv[1] : "config.yaml";
        if (!Config::Load(configPath)) {
            Logger::Error("Failed to load configuration from {}", configPath);
            return 1;
        }
        
        // Initialize database connection
        std::string dbHost = Config::GetString("database.host", "localhost");
        uint16_t dbPort = static_cast<uint16_t>(Config::GetInt("database.port", 5432));
        std::string dbName = Config::GetString("database.name", "kal_auth");
        std::string dbUser = Config::GetString("database.user", "kalonline");
        std::string dbPassword = Config::GetString("database.password", "");
        
        if (!Database::Initialize(dbHost, dbPort, dbName, dbUser, dbPassword)) {
            Logger::Error("Failed to initialize database connection");
            return 1;
        }
        
        Logger::Info("Database connection established");
        
        // Get server port from config
        uint16_t port = static_cast<uint16_t>(Config::GetInt("auth.port", 10001));
        
        // Create and start auth server
        asio::io_context ioContext;
        kal::auth::AuthServer authServer(ioContext, port);
        
        authServer.Start();
        
        Logger::Info("Auth Server is running. Press Ctrl+C to stop.");
        
        // Run event loop
        while (g_running) {
            ioContext.poll_one();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            // Periodic cleanup
            static auto lastCleanup = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::minutes>(now - lastCleanup).count() >= 5) {
                // Cleanup expired sessions could be called here if needed
                lastCleanup = now;
            }
        }
        
        // Graceful shutdown
        Logger::Info("Shutting down Auth Server...");
        authServer.Stop();
        Database::Shutdown();
        Logger::Info("Auth Server stopped gracefully");
        
        return 0;
        
    } catch (const std::exception& e) {
        Logger::Error("Fatal error: {}", e.what());
        return 1;
    }
}
