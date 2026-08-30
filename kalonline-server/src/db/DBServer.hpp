#pragma once

#include "common/network/Network.hpp"
#include "common/database/Database.hpp"

namespace kal::db {

class DBServer : public network::TcpServer {
public:
    explicit DBServer(asio::io_context& io_context, uint16_t port);
    
    void handle_query(network::ConnectionPtr conn, const network::Packet& packet);
};

} // namespace kal::db
