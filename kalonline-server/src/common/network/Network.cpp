#include "common/network/Network.hpp"
#include "common/crypto/Crypto.hpp"
#include <format>
#include <cstring>

namespace kal::network {

// ============================================================================
// ConnectionManager Implementation
// ============================================================================

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
    for (auto& [key, conn] : m_connections) {
        conn->disconnect();
    }
    m_connections.clear();
}

size_t ConnectionManager::connection_count() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connections.size();
}

// ============================================================================
// Connection Implementation
// ============================================================================

Connection::Connection(tcp::socket socket, ConnectionManager& manager)
    : m_socket(std::move(socket))
    , m_manager(manager)
    , m_read_buffer(HEADER_SIZE) {
}

Connection::~Connection() {
    disconnect();
}

std::string Connection::remote_endpoint() const {
    try {
        auto ep = m_socket.remote_endpoint();
        return std::format("{}:{}", ep.address().to_string(), ep.port());
    } catch (...) {
        return "unknown";
    }
}

asio::awaitable<void> Connection::start() {
    m_connected = true;
    m_manager.start(shared_from_this());
    
    KAL_LOG_INFO("New connection from {}", remote_endpoint());
    
    co_await asio::co_spawn(
        m_socket.get_executor(),
        read_loop(),
        asio::detached
    );
    
    co_await asio::co_spawn(
        m_socket.get_executor(),
        write_loop(),
        asio::detached
    );
    
    co_return;
}

asio::awaitable<void> Connection::read_loop() {
    while (m_connected) {
        try {
            // Read header (length + opcode)
            co_await read_header();
            
            if (!m_connected) break;
            
            // Parse header to get payload length
            uint16_t payload_length = (m_read_buffer[0] << 8) | m_read_buffer[1];
            
            // Validate payload length (prevent DoS)
            constexpr uint16_t MAX_PAYLOAD = 65535;
            if (payload_length > MAX_PAYLOAD) {
                KAL_LOG_WARNING("Invalid payload length {} from {}", payload_length, remote_endpoint());
                disconnect();
                break;
            }
            
            // Read payload
            co_await read_payload(payload_length);
            
            if (!m_connected) break;
            
            // Invoke receive handler with complete packet
            if (m_receive_handler) {
                m_receive_handler(std::span<const uint8_t>(m_read_buffer));
            }
            
        } catch (const asio::system_error& e) {
            if (e.code() != asio::error::operation_aborted) {
                KAL_LOG_DEBUG("Read error from {}: {}", remote_endpoint(), e.what());
            }
            disconnect();
            break;
        } catch (const std::exception& e) {
            KAL_LOG_ERROR("Read exception from {}: {}", remote_endpoint(), e.what());
            disconnect();
            break;
        }
    }
}

asio::awaitable<void> Connection::read_header() {
    m_read_buffer.resize(HEADER_SIZE);
    
    [[maybe_unused]] size_t n = co_await m_socket.async_read_some(
        asio::buffer(m_read_buffer.data(), HEADER_SIZE),
        asio::use_awaitable
    );
}

asio::awaitable<void> Connection::read_payload(size_t length) {
    if (length == 0) return;
    
    size_t offset = m_read_buffer.size();
    m_read_buffer.resize(offset + length);
    
    co_await asio::async_read(
        m_socket,
        asio::buffer(m_read_buffer.data() + offset, length),
        asio::use_awaitable
    );
}

asio::awaitable<void> Connection::write_loop() {
    while (m_connected) {
        std::vector<uint8_t> data_to_write;
        
        {
            std::lock_guard<std::mutex> lock(m_write_mutex);
            
            if (m_write_queue.empty()) {
                m_writing = false;
                
                // Wait for new data
                co_await asio::post(asio::use_awaitable);
                continue;
            }
            
            // Concatenate all queued writes for efficiency
            for (auto& buffer : m_write_queue) {
                data_to_write.insert(data_to_write.end(), buffer.begin(), buffer.end());
            }
            m_write_queue.clear();
        }
        
        try {
            co_await asio::async_write(
                m_socket,
                asio::buffer(data_to_write),
                asio::use_awaitable
            );
        } catch (const asio::system_error& e) {
            if (e.code() != asio::error::operation_aborted) {
                KAL_LOG_DEBUG("Write error to {}: {}", remote_endpoint(), e.what());
            }
            disconnect();
            break;
        }
    }
}

void Connection::send(std::span<const uint8_t> data) {
    send(data.data(), data.size());
}

void Connection::send(const uint8_t* data, size_t length) {
    if (!m_connected || length == 0) return;
    
    std::vector<uint8_t> buffer(data, data + length);
    
    {
        std::lock_guard<std::mutex> lock(m_write_mutex);
        m_write_queue.push_back(std::move(buffer));
    }
    
    // Wake up writer if not already running
    if (!m_writing) {
        m_socket.get_executor().post([self = shared_from_this()] {
            self->m_writing = true;
        });
    }
}

void Connection::disconnect() {
    bool expected = true;
    if (!m_connected.compare_exchange_strong(expected, false)) {
        return;  // Already disconnected
    }
    
    try {
        m_socket.close();
    } catch (...) {}
    
    m_manager.stop(shared_from_this());
    
    if (m_disconnect_handler) {
        m_disconnect_handler();
    }
    
    KAL_LOG_INFO("Disconnected {}", remote_endpoint());
}

// ============================================================================
// TcpServer Implementation
// ============================================================================

TcpServer::TcpServer(asio::io_context& io_context, uint16_t port)
    : m_io_context(io_context)
    , m_acceptor(io_context, tcp::endpoint(tcp::v4(), port))
    , m_port(port) {
}

void TcpServer::start() {
    m_running = true;
    
    asio::co_spawn(
        m_io_context,
        accept_loop(),
        asio::detached
    );
    
    KAL_LOG_INFO("TCP server listening on port {}", m_port);
}

void TcpServer::stop() {
    m_running = false;
    m_acceptor.close();
    
    KAL_LOG_INFO("TCP server stopped on port {}", m_port);
}

asio::awaitable<void> TcpServer::accept_loop() {
    while (m_running) {
        try {
            tcp::socket socket(m_io_context);
            co_await m_acceptor.async_accept(socket, asio::use_awaitable);
            
            if (!m_running) break;
            
            auto conn = create_connection(std::move(socket));
            
            if (m_manager) {
                m_manager->start(conn);
            }
            
            co_await asio::co_spawn(
                socket.get_executor(),
                handle_connection(conn),
                asio::detached
            );
            
        } catch (const asio::system_error& e) {
            if (e.code() != asio::error::operation_aborted && m_running) {
                KAL_LOG_ERROR("Accept error: {}", e.what());
            }
        } catch (const std::exception& e) {
            if (m_running) {
                KAL_LOG_ERROR("Accept exception: {}", e.what());
            }
        }
    }
}

asio::awaitable<void> TcpServer::handle_connection(ConnectionPtr conn) {
    co_await conn->start();
}

ConnectionPtr TcpServer::create_connection(tcp::socket socket) {
    return std::make_shared<Connection>(std::move(socket), 
                                        m_manager ? *m_manager : *(new ConnectionManager()));
}

// ============================================================================
// Packet Implementation
// ============================================================================

void Packet::serialize(std::vector<uint8_t>& buffer) const {
    buffer.resize(total_size());
    
    // Little-endian encoding
    buffer[0] = (length >> 8) & 0xFF;
    buffer[1] = length & 0xFF;
    buffer[2] = (opcode >> 8) & 0xFF;
    buffer[3] = opcode & 0xFF;
    
    if (!payload.empty()) {
        std::memcpy(buffer.data() + 4, payload.data(), payload.size());
    }
}

std::expected<Packet, std::string> Packet::deserialize(std::span<const uint8_t> data) {
    if (data.size() < 4) {
        return std::unexpected("Packet too small");
    }
    
    Packet pkt;
    pkt.length = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    pkt.opcode = (static_cast<uint16_t>(data[2]) << 8) | data[3];
    
    if (data.size() < 4 + pkt.payload.size()) {
        return std::unexpected("Incomplete packet payload");
    }
    
    if (pkt.length > 0) {
        pkt.payload.assign(data.begin() + 4, data.begin() + 4 + pkt.length);
    }
    
    return pkt;
}

} // namespace kal::network
