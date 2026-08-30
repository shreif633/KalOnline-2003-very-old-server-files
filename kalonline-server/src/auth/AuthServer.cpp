#include "AuthServer.hpp"
#include "AuthManager.hpp"
#include "Logger.hpp"
#include "Crypto.hpp"
#include <iostream>

namespace kal::auth {

AuthServer::AuthServer(asio::io_context& ioContext, uint16_t port)
    : ioContext_(ioContext)
    , acceptor_(ioContext, tcp::endpoint(tcp::v4(), port))
    , port_(port)
    , isRunning_(false)
{
    stats_ = {0, 0, 0, 0};
    Logger::Info("Auth Server initialized on port {}", port);
}

AuthServer::~AuthServer() {
    Stop();
}

void AuthServer::Start() {
    if (isRunning_) {
        Logger::Warn("Auth Server already running");
        return;
    }
    
    isRunning_ = true;
    Logger::Info("Auth Server starting...");
    DoAccept();
    Logger::Info("Auth Server started successfully on port {}", port_);
}

void AuthServer::Stop() {
    if (!isRunning_) return;
    
    isRunning_ = false;
    acceptor_.close();
    Logger::Info("Auth Server stopped");
}

AuthServer::Stats AuthServer::GetStatistics() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

size_t AuthServer::GetActiveSessionCount() const {
    return authManager_.GetActiveSessionCount();
}

void AuthServer::DoAccept() {
    if (!isRunning_) return;
    
    auto socket = std::make_shared<tcp::socket>(ioContext_);
    acceptor_.async_accept(*socket, [this, socket](std::error_code ec) {
        if (!ec) {
            HandleClient(socket);
        } else {
            if (isRunning_) {
                Logger::Error("Auth accept error: {}", ec.message());
            }
        }
        
        if (isRunning_) {
            DoAccept();
        }
    });
}

void AuthServer::HandleClient(std::shared_ptr<tcp::socket> socket) {
    {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_.currentConnections++;
    }
    
    Logger::Debug("New auth client connected from {}", 
                  socket->remote_endpoint().address().to_string());
    
    // Read packet header (length + opcode)
    auto headerBuffer = std::make_shared<std::vector<uint8_t>>(4);
    
    asio::async_read(*socket, asio::buffer(*headerBuffer),
        [this, socket, headerBuffer](std::error_code ec, size_t bytesTransferred) {
            if (ec) {
                Logger::Debug("Auth client disconnected (header read error)");
                {
                    std::lock_guard<std::mutex> lock(statsMutex_);
                    stats_.currentConnections--;
                }
                return;
            }
            
            // Parse header
            uint16_t length = (*headerBuffer)[0] | ((*headerBuffer)[1] << 8);
            uint16_t opcode = (*headerBuffer)[2] | ((*headerBuffer)[3] << 8);
            
            if (length > 65535 || length < 2) {
                Logger::Warn("Invalid packet length from auth client");
                socket->close();
                {
                    std::lock_guard<std::mutex> lock(statsMutex_);
                    stats_.currentConnections--;
                }
                return;
            }
            
            // Read payload
            auto payloadBuffer = std::make_shared<std::vector<uint8_t>>(length - 2);
            
            asio::async_read(*socket, asio::buffer(*payloadBuffer),
                [this, socket, headerBuffer, payloadBuffer, opcode](
                    std::error_code ec, size_t bytesTransferred) {
                    
                    if (ec) {
                        Logger::Debug("Auth client disconnected (payload read error)");
                        {
                            std::lock_guard<std::mutex> lock(statsMutex_);
                            stats_.currentConnections--;
                        }
                        return;
                    }
                    
                    // Combine header and payload for processing
                    std::vector<uint8_t> fullPacket;
                    fullPacket.insert(fullPacket.end(), headerBuffer->begin(), headerBuffer->end());
                    fullPacket.insert(fullPacket.end(), payloadBuffer->begin(), payloadBuffer->end());
                    
                    // Route to handler
                    switch (static_cast<AuthOpcode>(opcode)) {
                        case AuthOpcode::LoginRequest:
                            HandleLoginRequest(socket, fullPacket.data(), fullPacket.size());
                            break;
                        case AuthOpcode::LogoutRequest:
                            HandleLogoutRequest(socket, fullPacket.data(), fullPacket.size());
                            break;
                        case AuthOpcode::SessionValidate:
                            HandleSessionValidate(socket, fullPacket.data(), fullPacket.size());
                            break;
                        default:
                            Logger::Warn("Unknown auth opcode: 0x{:04X}", opcode);
                            break;
                    }
                    
                    // Continue listening for more packets from same client
                    HandleClient(socket);
                });
        });
}

void AuthServer::HandleLoginRequest(std::shared_ptr<tcp::socket> socket,
                                    const uint8_t* data, size_t length) {
    std::lock_guard<std::mutex> lock(statsMutex_);
    stats_.totalLogins++;
    
    // Parse login packet: [length][opcode][username_len][username][password_len][password][ip_len][ip]
    if (length < 10) {
        Logger::Warn("Invalid login packet size");
        stats_.failedLogins++;
        return;
    }
    
    try {
        size_t offset = 4; // Skip header
        
        uint8_t usernameLen = data[offset++];
        std::string username(reinterpret_cast<const char*>(&data[offset]), usernameLen);
        offset += usernameLen;
        
        uint8_t passwordLen = data[offset++];
        std::string password(reinterpret_cast<const char*>(&data[offset]), passwordLen);
        offset += passwordLen;
        
        uint8_t ipLen = data[offset++];
        std::string ip(reinterpret_cast<const char*>(&data[offset]), ipLen);
        
        Logger::Info("Login attempt for user '{}' from {}", username, ip);
        
        std::string token;
        uint32_t userId = 0;
        
        auto result = authManager_.Authenticate(username, password, ip, token, userId);
        
        if (result == AuthManager::LoginResult::Success) {
            stats_.successfulLogins++;
            Logger::Info("Login successful for user {} (ID: {})", username, userId);
        } else {
            stats_.failedLogins++;
            Logger::Warn("Login failed for user {}: {:?}", username, static_cast<int>(result));
        }
        
        auto response = BuildLoginResponse(result, token, userId);
        asio::async_write(*socket, asio::buffer(response),
            [socket](std::error_code ec, size_t) {
                if (ec) {
                    Logger::Debug("Failed to send login response");
                }
            });
        
    } catch (const std::exception& e) {
        Logger::Error("Error handling login request: {}", e.what());
        stats_.failedLogins++;
    }
}

void AuthServer::HandleLogoutRequest(std::shared_ptr<tcp::socket> socket,
                                     const uint8_t* data, size_t length) {
    if (length < 6) return; // Minimum: header + token_len
    
    try {
        size_t offset = 4;
        uint8_t tokenLen = data[offset++];
        std::string token(reinterpret_cast<const char*>(&data[offset]), tokenLen);
        
        bool success = authManager_.Logout(token);
        auto response = BuildLogoutResponse(success);
        
        asio::async_write(*socket, asio::buffer(response),
            [socket](std::error_code ec, size_t) {});
        
        Logger::Debug("Logout request processed: {}", success ? "success" : "failed");
        
    } catch (const std::exception& e) {
        Logger::Error("Error handling logout request: {}", e.what());
    }
}

void AuthServer::HandleSessionValidate(std::shared_ptr<tcp::socket> socket,
                                       const uint8_t* data, size_t length) {
    if (length < 6) return;
    
    try {
        size_t offset = 4;
        uint8_t tokenLen = data[offset++];
        std::string token(reinterpret_cast<const char*>(&data[offset]), tokenLen);
        
        bool valid = authManager_.ValidateSession(token);
        auto response = BuildSessionValidateResponse(valid);
        
        asio::async_write(*socket, asio::buffer(response),
            [socket](std::error_code ec, size_t) {});
        
    } catch (const std::exception& e) {
        Logger::Error("Error validating session: {}", e.what());
    }
}

void AuthServer::HandleServerListRequest(std::shared_ptr<tcp::socket> socket,
                                         const uint8_t* data, size_t length) {
    auto response = BuildServerListResponse();
    
    asio::async_write(*socket, asio::buffer(response),
        [socket](std::error_code ec, size_t) {});
}

std::vector<uint8_t> AuthServer::BuildLoginResponse(AuthManager::LoginResult result,
                                                     const std::string& token,
                                                     uint32_t userId) {
    std::vector<uint8_t> response;
    
    // Result code (1 byte)
    uint8_t resultCode = 0;
    switch (result) {
        case AuthManager::LoginResult::Success: resultCode = 0; break;
        case AuthManager::LoginResult::InvalidCredentials: resultCode = 1; break;
        case AuthManager::LoginResult::AccountLocked: resultCode = 2; break;
        case AuthManager::LoginResult::AlreadyLoggedIn: resultCode = 3; break;
        case AuthManager::LoginResult::ServerFull: resultCode = 4; break;
        case AuthManager::LoginResult::DatabaseError: resultCode = 5; break;
    }
    
    // Build packet: [length][opcode][resultCode][token_len][token][user_id]
    response.push_back(0); // Length placeholder (low byte)
    response.push_back(0); // Length placeholder (high byte)
    response.push_back(static_cast<uint8_t>(AuthOpcode::LoginResponse)); // Opcode low
    response.push_back(static_cast<uint8_t>(static_cast<uint16_t>(AuthOpcode::LoginResponse) >> 8)); // Opcode high
    
    response.push_back(resultCode);
    
    if (result == AuthManager::LoginResult::Success) {
        response.push_back(static_cast<uint8_t>(token.length()));
        response.insert(response.end(), token.begin(), token.end());
        response.push_back(static_cast<uint8_t>(userId & 0xFF));
        response.push_back(static_cast<uint8_t>((userId >> 8) & 0xFF));
        response.push_back(static_cast<uint8_t>((userId >> 16) & 0xFF));
        response.push_back(static_cast<uint8_t>((userId >> 24) & 0xFF));
    }
    
    // Set length
    uint16_t totalLength = static_cast<uint16_t>(response.size());
    response[0] = static_cast<uint8_t>(totalLength & 0xFF);
    response[1] = static_cast<uint8_t>((totalLength >> 8) & 0xFF);
    
    return response;
}

std::vector<uint8_t> AuthServer::BuildLogoutResponse(bool success) {
    std::vector<uint8_t> response(5);
    
    response[0] = 5; // Length
    response[1] = 0;
    response[2] = static_cast<uint8_t>(AuthOpcode::LogoutResponse);
    response[3] = 0;
    response[4] = success ? 1 : 0;
    
    return response;
}

std::vector<uint8_t> AuthServer::BuildSessionValidateResponse(bool valid) {
    std::vector<uint8_t> response(5);
    
    response[0] = 5;
    response[1] = 0;
    response[2] = static_cast<uint8_t>(AuthOpcode::SessionValidateResponse);
    response[3] = 0;
    response[4] = valid ? 1 : 0;
    
    return response;
}

std::vector<uint8_t> AuthServer::BuildServerListResponse() {
    std::vector<uint8_t> response;
    
    // Simple server list with one server (MainSvr)
    // Format: [length][opcode][server_count][server_info...]
    response.push_back(0); // Length placeholder
    response.push_back(0);
    response.push_back(static_cast<uint8_t>(AuthOpcode::ServerListResponse));
    response.push_back(0);
    
    response.push_back(1); // 1 server
    
    // Server info: [id][name_len][name][ip_len][ip][port][status][population]
    response.push_back(1); // Server ID
    std::string serverName = "KalOnline";
    response.push_back(static_cast<uint8_t>(serverName.length()));
    response.insert(response.end(), serverName.begin(), serverName.end());
    
    std::string serverIp = "127.0.0.1";
    response.push_back(static_cast<uint8_t>(serverIp.length()));
    response.insert(response.end(), serverIp.begin(), serverIp.end());
    
    uint16_t serverPort = 10003; // Main server port
    response.push_back(static_cast<uint8_t>(serverPort & 0xFF));
    response.push_back(static_cast<uint8_t>((serverPort >> 8) & 0xFF));
    
    response.push_back(1); // Status: Online
    response.push_back(0); // Population: Low
    
    // Set length
    uint16_t totalLength = static_cast<uint16_t>(response.size());
    response[0] = static_cast<uint8_t>(totalLength & 0xFF);
    response[1] = static_cast<uint8_t>((totalLength >> 8) & 0xFF);
    
    return response;
}

} // namespace kal::auth
