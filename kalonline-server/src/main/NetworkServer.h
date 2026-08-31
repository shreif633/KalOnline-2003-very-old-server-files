#pragma once

#include "common/network/Network.hpp"
#include <asio.hpp>
#include <memory>
#include <functional>
#include <atomic>

namespace kal {

using NetworkSession = network::Connection;
using NetworkPacketCallback = std::function<void(const std::shared_ptr<NetworkSession>&, uint16_t opcode, std::span<const uint8_t> payload)>;

/**
 * @brief Network server wrapper for game server
 */
class NetworkServer {
public:
    explicit NetworkServer(asio::io_context& io_context, uint16_t port);
    ~NetworkServer();

    void start();
    void stop();

    bool is_running() const { return running_; }

    void set_packet_handler(NetworkPacketCallback callback);

    void send_to_session(const std::shared_ptr<NetworkSession>& session, 
                         const std::vector<uint8_t>& packet);

    size_t connection_count() const;

private:
    asio::io_context& io_context_;
    uint16_t port_;
    std::unique_ptr<network::TcpServer> tcp_server_;
    std::atomic<bool> running_{false};
    NetworkPacketCallback packet_handler_;
};

} // namespace kal
