#pragma once
#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include <asio.hpp>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace kal::auth {

using namespace kal::network;
using namespace kal::database;

struct AuthStats {
    uint64_t totalLogins;
    uint64_t failedLogins;
    uint64_t activeSessions;
    uint64_t blockedIPs;
};

class AuthServer {
public:
    AuthServer(asio::io_context& ioContext, uint16_t port);
    ~AuthServer();

    void Start();
    void Stop();
    
    AuthStats GetStats() const;

private:
    void DoAccept();
    void OnClientConnected(std::shared_ptr<TcpConnection> connection);
    
    // Packet handlers
    void HandleLogin(std::shared_ptr<TcpConnection> conn, std::span<const uint8_t> packet);
    void HandleRegister(std::shared_ptr<TcpConnection> conn, std::span<const uint8_t> packet);
    void HandleServerList(std::shared_ptr<TcpConnection> conn, std::span<const uint8_t> packet);
    
    // Session management
    bool ValidateSession(const std::string& sessionToken);
    std::string CreateSession(const std::string& username);
    void RemoveSession(const std::string& sessionToken);

    asio::io_context& ioContext_;
    tcp::acceptor acceptor_;
    uint16_t port_;
    std::atomic<bool> isRunning_;
    
    mutable std::mutex sessionMutex_;
    std::unordered_map<std::string, std::string> sessions_; // token -> username
    
    mutable std::mutex statsMutex_;
    AuthStats stats_;
};

} // namespace kal::auth
