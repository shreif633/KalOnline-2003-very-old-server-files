#include "common/network/Network.hpp"
#include "common/crypto/Crypto.hpp"
#include <fmt/format.h>
#include <cstring>
#include <sstream>

namespace kal::network {

// ConnectionManager Implementation
void ConnectionManager::start(ConnectionPtr conn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.insert({conn.get(), conn});
}

void ConnectionManager::stop(ConnectionPtr conn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.erase(conn.get());
}

void ConnectionManager::stop_all() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_connections) {
        if (pair.second) {
            pair.second->disconnect();
        }
    }
    m_connections.clear();
}

size_t ConnectionManager::connection_count() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connections.size();
}

// Connection Implementation
Connection::Connection(tcp::socket socket, ConnectionManager& manager)
    : m_socket(std::move(socket))
    , m_manager(manager)
    , m_connected(true)
    , m_writing(false) {
}

Connection::~Connection() {
    disconnect();
}

std::string Connection::remote_endpoint() const {
    try {
        auto ep = m_socket.remote_endpoint();
        return fmt::format("{}:{}", ep.address().to_string(), ep.port());
    } catch (...) {
        return "unknown";
    }
}

void Connection::start() {
    m_manager.start(shared_from_this());
    do_read_header();
}

void Connection::do_read_header() {
    auto self = shared_from_this();
    asio::async_read(m_socket, asio::buffer(m_read_header_buffer),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (!ec && m_connected) {
                uint16_t payload_length = (static_cast<uint16_t>(m_read_header_buffer[0]) << 8) | 
                                          m_read_header_buffer[1];
                
                if (payload_length > 65535) {
                    disconnect();
                    return;
                }
                
                m_read_payload.resize(payload_length);
                do_read_payload(payload_length);
            } else {
                disconnect();
            }
        });
}

void Connection::do_read_payload(size_t length) {
    auto self = shared_from_this();
    asio::async_read(m_socket, asio::buffer(m_read_payload),
        [this, self, length](std::error_code ec, std::size_t /*bytes_transferred*/) {
            if (!ec && m_connected) {
                if (m_packet_handler) {
                    uint16_t opcode = (static_cast<uint16_t>(m_read_payload[0]) << 8) | 
                                      m_read_payload[1];
                    
                    if (m_read_payload.size() >= 2) {
                        std::span<const uint8_t> payload(m_read_payload.data() + 2, 
                                                         m_read_payload.size() - 2);
                        m_packet_handler(self, opcode, payload);
                    }
                }
                do_read_header();
            } else {
                disconnect();
            }
        });
}

void Connection::send(std::span<const uint8_t> data) {
    if (!m_connected || data.empty()) {
        return;
    }
    
    bool expected = false;
    if (!m_writing.compare_exchange_strong(expected, true)) {
        std::lock_guard<std::mutex> lock(m_write_queue_mutex);
        m_write_queue.emplace(data.begin(), data.end());
        return;
    }
    
    m_current_write.assign(data.begin(), data.end());
    do_write();
}

void Connection::do_write() {
    auto self = shared_from_this();
    asio::async_write(m_socket, asio::buffer(m_current_write),
        [this, self](std::error_code ec, std::size_t /*bytes_transferred*/) {
            if (!ec && m_connected) {
                bool expected = true;
                if (m_writing.compare_exchange_strong(expected, false)) {
                    std::lock_guard<std::mutex> lock(m_write_queue_mutex);
                    if (!m_write_queue.empty()) {
                        m_current_write = std::move(m_write_queue.front());
                        m_write_queue.pop();
                        expected = false;
                        if (m_writing.compare_exchange_strong(expected, true)) {
                            do_write();
                        }
                    }
                }
            } else {
                disconnect();
            }
        });
}

void Connection::disconnect() {
    if (!m_connected.exchange(false)) {
        return;
    }
    
    std::error_code ec;
    m_socket.shutdown(tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
    m_manager.stop(shared_from_this());
}

// TcpServer Implementation
TcpServer::TcpServer(asio::io_context& io_context, uint16_t port)
    : m_io_context(io_context)
    , m_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
    , m_port(port)
    , m_manager(nullptr) {
}

void TcpServer::start() {
    do_accept();
}

void TcpServer::stop() {
    std::error_code ec;
    m_acceptor.close(ec);
    if (m_manager) {
        m_manager->stop_all();
    }
}

void TcpServer::do_accept() {
    m_acceptor.async_accept(
        [this](std::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto conn = create_connection(std::move(socket));
                if (conn) {
                    handle_connection(conn);
                }
            }
            do_accept();
        });
}

ConnectionPtr TcpServer::create_connection(tcp::socket socket) {
    if (!m_manager) {
        return nullptr;
    }
    return std::make_shared<Connection>(std::move(socket), *m_manager);
}

void TcpServer::handle_connection(ConnectionPtr conn) {
    conn->set_packet_handler(m_packet_handler);
    conn->start();
}

void TcpServer::set_packet_handler(PacketCallback callback) {
    m_packet_handler = std::move(callback);
}

size_t TcpServer::connection_count() const noexcept {
    if (!m_manager) {
        return 0;
    }
    return m_manager->connection_count();
}

} // namespace kal::network
