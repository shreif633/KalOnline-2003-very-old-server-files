#include "db/DBServer.hpp"

namespace kal::db {

DBServer::DBServer(asio::io_context& io_context, uint16_t port)
    : network::TcpServer(io_context, port) {
}

void DBServer::handle_query(network::ConnectionPtr conn, const network::Packet& packet) {
    // Parse query request
    // Route to appropriate handler
    // Return result
}

} // namespace kal::db
