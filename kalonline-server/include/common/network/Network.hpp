#pragma once

#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/use_awaitable.hpp>

#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <atomic>

#include "common/logger/Logger.hpp"

namespace kal::network {

using asio::ip::tcp;

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;
using ConnectionWeakPtr = std::weak_ptr<Connection>;

/**
 * @brief Thread-safe connection manager
 * Tracks all active connections and provides cleanup on shutdown
 */
class ConnectionManager {
public:
    void start(ConnectionPtr conn);
    void stop(ConnectionPtr conn);
    void stop_all();
    
    [[nodiscard]] size_t connection_count() const noexcept;
    
private:
    std::unordered_map<Connection*, ConnectionPtr> m_connections;
    mutable std::mutex m_mutex;
};

/**
 * @brief Single TCP connection handler using C++23 coroutines
 */
class Connection : public std::enable_shared_from_this<Connection> {
public:
    using ReceiveHandler = std::function<void(std::span<const uint8_t>)>;
    using DisconnectHandler = std::function<void()>;
    
    explicit Connection(tcp::socket socket, ConnectionManager& manager);
    ~Connection();
    
    [[nodiscard]] tcp::socket& socket() noexcept { return m_socket; }
    [[nodiscard]] const tcp::socket& socket() const noexcept { return m_socket; }
    [[nodiscard]] std::string remote_endpoint() const;
    
    // Coroutine-based async operations
    asio::awaitable<void> start();
    asio::awaitable<void> read_loop();
    asio::awaitable<void> write_loop();
    
    // Send data (thread-safe)
    void send(std::span<const uint8_t> data);
    void send(const uint8_t* data, size_t length);
    
    // Handlers
    void set_receive_handler(ReceiveHandler handler) { m_receive_handler = std::move(handler); }
    void set_disconnect_handler(DisconnectHandler handler) { m_disconnect_handler = std::move(handler); }
    
    [[nodiscard]] bool is_connected() const noexcept { return m_connected; }
    void disconnect();
    
private:
    asio::awaitable<void> read_header();
    asio::awaitable<void> read_payload(size_t length);
    
    tcp::socket m_socket;
    ConnectionManager& m_manager;
    
    // Read buffer with header-first approach
    std::vector<uint8_t> m_read_buffer;
    static constexpr size_t HEADER_SIZE = 4;  // 2 bytes length + 2 bytes opcode
    
    // Write queue with mutex protection
    std::deque<std::vector<uint8_t>> m_write_queue;
    mutable std::mutex m_write_mutex;
    bool m_writing{false};
    
    std::atomic<bool> m_connected{false};
    
    ReceiveHandler m_receive_handler;
    DisconnectHandler m_disconnect_handler;
};

/**
 * @brief Async TCP server accepting connections on specified port
 */
class TcpServer {
public:
    explicit TcpServer(asio::io_context& io_context, uint16_t port);
    
    void start();
    void stop();
    
    void set_connection_manager(ConnectionManager& manager) { m_manager = &manager; }
    void set_connection_factory(std::function<ConnectionPtr(tcp::socket)> factory) {
        m_connection_factory = std::move(factory);
    }
    
    [[nodiscard]] uint16_t port() const noexcept { return m_port; }
    [[nodiscard]] bool is_running() const noexcept { return m_running; }
    
protected:
    virtual asio::awaitable<void> handle_connection(ConnectionPtr conn);
    virtual ConnectionPtr create_connection(tcp::socket socket);
    
private:
    asio::awaitable<void> accept_loop();
    
    asio::io_context& m_io_context;
    tcp::acceptor m_acceptor;
    uint16_t m_port;
    std::atomic<bool> m_running{false};
    
    ConnectionManager* m_manager{nullptr};
    std::function<ConnectionPtr(tcp::socket)> m_connection_factory;
};

/**
 * @brief Packet structure for KalOnline protocol
 * Original 2003 protocol format: [Length:2][Opcode:2][Payload:N]
 */
struct Packet {
    uint16_t length;
    uint16_t opcode;
    std::vector<uint8_t> payload;
    
    [[nodiscard]] size_t total_size() const noexcept {
        return sizeof(length) + sizeof(opcode) + payload.size();
    }
    
    void serialize(std::vector<uint8_t>& buffer) const;
    [[nodiscard]] static std::expected<Packet, std::string> deserialize(std::span<const uint8_t> data);
};

} // namespace kal::network
