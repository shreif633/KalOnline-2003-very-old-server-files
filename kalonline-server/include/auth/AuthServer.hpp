#pragma once
#include "common/network/Network.hpp"
#include <optional>
#include <shared_mutex>
#include <unordered_map>
namespace kal::auth {
struct Session { uint32_t session_id; uint32_t user_id; std::string username; };
class AuthServer : public network::TcpServer {
public:
    explicit AuthServer(asio::io_context& io_context, uint16_t port);
    void handle_login(network::ConnectionPtr conn, const network::Packet& packet);
private:
    std::unordered_map<uint32_t, Session> m_sessions;
    mutable std::shared_mutex m_sessions_mutex;
};
}
