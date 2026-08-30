#include "auth/AuthServer.hpp"

namespace kal::auth {

AuthServer::AuthServer(asio::io_context& io_context, uint16_t port)
    : network::TcpServer(io_context, port) {
}

void AuthServer::handle_login(ConnectionPtr conn, const Packet& packet) {
    // Parse login credentials
    // Verify against database with BCrypt
    // Create session on success
}

void AuthServer::handle_register(ConnectionPtr conn, const Packet& packet) {
    // Create new account
    // Hash password with BCrypt
    // Store in database
}

void AuthServer::handle_session_validate(ConnectionPtr conn, const Packet& packet) {
    // Validate session token
    // Return user info if valid
}

} // namespace kal::auth
