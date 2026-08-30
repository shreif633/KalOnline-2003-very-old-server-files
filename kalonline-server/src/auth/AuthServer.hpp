#pragma once

#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include <unordered_map>
#include <shared_mutex>

namespace kal::auth {

struct Session {
    uint32_t session_id;
    uint32_t user_id;
    std::string username;
    std::chrono::steady_clock::time_point created_at;
    std::chrono::steady_clock::time_point expires_at;
    std::string remote_ip;
};

class AuthServer : public network::TcpServer {
public:
    explicit AuthServer(asio::io_context& io_context, uint16_t port);
    
    void handle_login(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_register(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_session_validate(network::ConnectionPtr conn, const network::Packet& packet);
    
private:
    [[nodiscard]] std::optional<Session> create_session(uint32_t user_id, 
                                                         const std::string& username,
                                                         const std::string& ip);
    void cleanup_expired_sessions();
    
    std::unordered_map<uint32_t, Session> m_sessions;
    mutable std::shared_mutex m_sessions_mutex;
    uint32_t m_next_session_id{1};
};

} // namespace kal::auth
