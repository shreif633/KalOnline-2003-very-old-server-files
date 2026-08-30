#include "AuthServer.hpp"
#include "auth/core/AuthManager.hpp"
#include "common/logger/Logger.hpp"
#include "common/crypto/Crypto.hpp"
#include <iostream>
#include <random>

namespace kal::auth {

using namespace kal::network;
using namespace kal::database;
using namespace kal::logger;

AuthServer::AuthServer(asio::io_context& ioContext, uint16_t port)
    : ioContext_(ioContext)
    , acceptor_(ioContext, tcp::endpoint(tcp::v4(), port))
    , port_(port)
    , isRunning_(false)
{
    stats_ = AuthStats{0, 0, 0, 0};
    KAL_LOG_INFO("AuthServer created on port {}", port);
}

AuthServer::~AuthServer() {
    Stop();
}

void AuthServer::Start() {
    if (isRunning_.exchange(true)) {
        return;
    }
    
    KAL_LOG_INFO("AuthServer starting...");
    DoAccept();
}

void AuthServer::Stop() {
    if (!isRunning_.exchange(false)) {
        return;
    }
    
    KAL_LOG_INFO("AuthServer stopping...");
    acceptor_.close();
}

AuthStats AuthServer::GetStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

void AuthServer::DoAccept() {
    if (!isRunning_) {
        return;
    }
    
    acceptor_.async_accept(
        [this](asio::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto connection = std::make_shared<Connection>(std::move(socket), connectionManager_);
                OnClientConnected(connection);
            } else {
                if (isRunning_) {
                    KAL_LOG_ERROR("Accept error: {}", ec.message());
                }
            }
            
            if (isRunning_) {
                DoAccept();
            }
        });
}

void AuthServer::OnClientConnected(std::shared_ptr<kal::network::Connection> connection) {
    KAL_LOG_DEBUG("Client connected from {}", connection->remote_endpoint());
    
    connection->set_packet_handler(
        [this, weakConn = std::weak_ptr<Connection>(connection)](
            kal::network::ConnectionPtr conn, uint16_t opcode, std::span<const uint8_t> packet) mutable {
            auto connection = weakConn.lock();
            if (connection) {
                OnPacketReceived(connection, packet);
            }
        });
    
    connection->start();
}

void AuthServer::OnPacketReceived(std::shared_ptr<kal::network::Connection> conn, std::span<const uint8_t> packet) {
    if (packet.size() < 4) {
        KAL_LOG_WARNING("Invalid packet size: {}", packet.size());
        return;
    }
    
    uint16_t length = static_cast<uint16_t>(packet[0]) | (static_cast<uint16_t>(packet[1]) << 8);
    uint16_t opcode = static_cast<uint16_t>(packet[2]) | (static_cast<uint16_t>(packet[3]) << 8);
    
    KAL_LOG_DEBUG("Received packet: Opcode=0x{:04X}, Length={}", opcode, length);
    
    switch (opcode) {
        case 0x1001:
            HandleLogin(conn, packet.subspan(4));
            break;
        case 0x1002:
            HandleRegister(conn, packet.subspan(4));
            break;
        case 0x1003:
            HandleServerList(conn, packet.subspan(4));
            break;
        default:
            KAL_LOG_DEBUG("Unhandled opcode: 0x{:04X}", opcode);
            break;
    }
}

void AuthServer::HandleLogin(std::shared_ptr<kal::network::Connection> conn, std::span<const uint8_t> packet) {
    KAL_LOG_DEBUG("Handling login request");
    // TODO: Parse username/password, validate, send response
}

void AuthServer::HandleRegister(std::shared_ptr<kal::network::Connection> conn, std::span<const uint8_t> packet) {
    KAL_LOG_DEBUG("Handling register request");
    // TODO: Parse registration data, create account, send response
}

void AuthServer::HandleServerList(std::shared_ptr<kal::network::Connection> conn, std::span<const uint8_t> packet) {
    KAL_LOG_DEBUG("Handling server list request");
    // TODO: Send server list response
}

bool AuthServer::ValidateSession(const std::string& sessionToken) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return sessions_.find(sessionToken) != sessions_.end();
}

std::string AuthServer::CreateSession(const std::string& username) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string token;
    token.reserve(32);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61);
    
    for (int i = 0; i < 32; ++i) {
        token += chars[dis(gen)];
    }
    
    sessions_[token] = username;
    return token;
}

void AuthServer::RemoveSession(const std::string& sessionToken) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    sessions_.erase(sessionToken);
}

} // namespace kal::auth
