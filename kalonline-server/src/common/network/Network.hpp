#pragma once

#include <asio.hpp>
#include <memory>
#include <span>
#include <vector>
#include <deque>
#include <array>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <expected>

namespace kal::network {

using tcp = asio::ip::tcp;

// Forward declarations
class Connection;
class ConnectionManager;

using ConnectionPtr = std::shared_ptr<Connection>;
using PacketCallback = std::function<void(ConnectionPtr, uint16_t opcode, std::span<const uint8_t> payload)>;

class ConnectionManager {
public:
    void start(ConnectionPtr conn);
    void stop(ConnectionPtr conn);
    void stop_all();
    size_t connection_count() const noexcept;

private:
    mutable std::mutex m_mutex;
    std::unordered_map<Connection*, ConnectionPtr> m_connections;
};

class Connection : public std::enable_shared_from_this<Connection> {
public:
    explicit Connection(tcp::socket socket, ConnectionManager& manager);
    ~Connection();

    std::string remote_endpoint() const;
    void start();
    
    void disconnect();
    void send(std::span<const uint8_t> data);
    void set_packet_handler(PacketCallback callback) { m_packet_handler = std::move(callback); }

private:
    void do_read_header();
    void do_read_payload(size_t length);
    void do_write();
    
    tcp::socket m_socket;
    ConnectionManager& m_manager;
    
    std::array<uint8_t, 2> m_read_header_buffer;
    std::vector<uint8_t> m_read_payload;
    std::deque<std::vector<uint8_t>> m_write_queue;
    
    std::atomic<bool> m_connected = false;
    std::atomic<bool> m_writing = false;
    
    PacketCallback m_packet_handler;
    
    mutable std::mutex m_write_queue_mutex;
    
    static constexpr size_t HEADER_SIZE = 2;
};

class TcpServer {
public:
    explicit TcpServer(asio::io_context& io_context, uint16_t port);
    
    void start();
    void stop();
    
    void set_connection_manager(ConnectionManager* manager) { m_manager = manager; }
    void set_packet_handler(PacketCallback callback);
    size_t connection_count() const noexcept;

protected:
    virtual ConnectionPtr create_connection(tcp::socket socket);
    virtual void handle_connection(ConnectionPtr conn);

    asio::io_context& m_io_context;
    tcp::acceptor m_acceptor;
    uint16_t m_port;
    bool m_running = false;
    ConnectionManager* m_manager = nullptr;
    PacketCallback m_packet_handler;

private:
    void do_accept();
};

struct Packet {
    uint16_t length = 0;
    uint16_t opcode = 0;
    std::vector<uint8_t> payload;
    
    size_t total_size() const { return 4 + payload.size(); }
    
    void serialize(std::vector<uint8_t>& buffer) const;
    static std::expected<Packet, std::string> deserialize(std::span<const uint8_t> data);
};

} // namespace kal::network
