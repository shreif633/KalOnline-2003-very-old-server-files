#include "auth/AuthServer.hpp"
#include "common/logger/Logger.hpp"
#include <bcrypt/bcrypt.h>
#include <uuid.h>
#include <fmt/format.h>
#include <regex>

namespace kal::auth {

using namespace network;
using namespace database;
using namespace logger;

AuthServer::AuthServer(asio::io_context& io_context, uint16_t port,
                       std::shared_ptr<DatabasePool> db_pool)
    : TcpServer(io_context, port)
    , m_db_pool(std::move(db_pool))
    , m_cleanup_timer(io_context)
{
    m_stats.start_time = std::chrono::system_clock::now();
    LOG_INFO("AuthServer initialized on port {}", port);
}

AuthServer::~AuthServer() {
    stop();
}

void AuthServer::start() {
    if (m_running.exchange(true)) {
        LOG_WARN("AuthServer already running");
        return;
    }
    
    LOG_INFO("Starting AuthServer...");
    TcpServer::start();
    
    // Start periodic cleanup of expired sessions
    schedule_cleanup();
    
    LOG_INFO("AuthServer started successfully");
}

void AuthServer::stop() {
    if (!m_running.exchange(false)) {
        return;
    }
    
    LOG_INFO("Stopping AuthServer...");
    
    // Remove all sessions
    {
        std::unique_lock lock(m_sessions_mutex);
        m_sessions.clear();
        m_user_to_session.clear();
    }
    
    m_cleanup_timer.cancel();
    TcpServer::stop();
    
    LOG_INFO("AuthServer stopped");
}

void AuthServer::on_packet_received(ConnectionPtr conn, const Packet& packet) {
    if (packet.size() < sizeof(uint16_t)) {
        LOG_WARN("Invalid packet received from {}: too small", 
                 conn->get_endpoint());
        conn->close();
        return;
    }
    
    // Parse opcode
    const auto opcode = *reinterpret_cast<const AuthOpcode*>(packet.data());
    
    LOG_DEBUG("Received auth packet: opcode={:#06x}, size={}", 
              static_cast<uint16_t>(opcode), packet.size());
    
    switch (opcode) {
        case AuthOpcode::LoginRequest:
            handle_login(conn, packet);
            break;
        case AuthOpcode::RegisterRequest:
            handle_register(conn, packet);
            break;
        case AuthOpcode::SessionValidate:
            handle_session_validate(conn, packet);
            break;
        case AuthOpcode::LogoutRequest:
            handle_logout(conn, packet);
            break;
        case AuthOpcode::SessionQuery:
            handle_session_query(conn, packet);
            break;
        default:
            LOG_WARN("Unknown auth opcode: {:#06x}", 
                     static_cast<uint16_t>(opcode));
            break;
    }
}

void AuthServer::handle_login(ConnectionPtr conn, const Packet& packet) {
    m_stats.total_logins.fetch_add(1, std::memory_order_relaxed);
    
    // Check rate limiting
    const auto ip = conn->get_endpoint();
    {
        std::unique_lock lock(m_rate_limit_mutex);
        auto now = std::chrono::steady_clock::now();
        auto it = m_login_attempts.find(ip);
        
        if (it != m_login_attempts.end()) {
            // Count attempts in the last LOGIN_WINDOW
            auto window_start = now - LOGIN_WINDOW;
            if (it->second >= window_start) {
                // Too many attempts
                m_stats.failed_logins.fetch_add(1, std::memory_order_relaxed);
                auto response = create_login_response(LoginResult::AccountLocked);
                conn->send(response);
                LOG_WARN("Rate limit exceeded for IP: {}", ip);
                return;
            }
        }
        
        // Record this attempt
        m_login_attempts[ip] = now;
    }
    
    // Check if IP is blocked
    if (is_ip_blocked(ip)) {
        m_stats.failed_logins.fetch_add(1, std::memory_order_relaxed);
        auto response = create_login_response(LoginResult::AccountBanned);
        conn->send(response);
        conn->close();
        return;
    }
    
    // Parse login request
    // Packet format: [Opcode:2][UsernameLen:1][Username:N][PasswordLen:1][Password:M][ClientVer:4]
    if (packet.size() < sizeof(AuthOpcode) + 2) {
        auto response = create_login_response(LoginResult::InvalidCredentials);
        conn->send(response);
        return;
    }
    
    size_t offset = sizeof(AuthOpcode);
    uint8_t username_len = packet[offset++];
    
    if (offset + username_len > packet.size()) {
        auto response = create_login_response(LoginResult::InvalidCredentials);
        conn->send(response);
        return;
    }
    
    std::string username(packet.data() + offset, username_len);
    offset += username_len;
    
    if (offset >= packet.size()) {
        auto response = create_login_response(LoginResult::InvalidCredentials);
        conn->send(response);
        return;
    }
    
    uint8_t password_len = packet[offset++];
    
    if (offset + password_len > packet.size()) {
        auto response = create_login_response(LoginResult::InvalidCredentials);
        conn->send(response);
        return;
    }
    
    std::string password(packet.data() + offset, password_len);
    offset += password_len;
    
    // Parse client version if present
    std::string client_version = "unknown";
    uint32_t client_build = 0;
    if (offset + sizeof(uint32_t) <= packet.size()) {
        client_build = *reinterpret_cast<const uint32_t*>(packet.data() + offset);
        client_version = fmt::format("build_{}", client_build);
    }
    
    LOG_INFO("Login attempt: user={}, ip={}, client={}", 
             username, ip, client_version);
    
    // Process login
    std::string session_token;
    auto result = process_login({username, password, client_version, client_build}, 
                                session_token);
    
    if (result == LoginResult::Success) {
        m_stats.active_sessions.fetch_add(1, std::memory_order_relaxed);
        LOG_INFO("Login successful: user={}, session={}", 
                 username, session_token.substr(0, 8));
    } else {
        m_stats.failed_logins.fetch_add(1, std::memory_order_relaxed);
        LOG_WARN("Login failed: user={}, reason={}", 
                 static_cast<int>(result), username);
    }
    
    auto response = create_login_response(result, session_token);
    conn->send(response);
}

void AuthServer::handle_register(ConnectionPtr conn, const Packet& packet) {
    // Parse registration request
    // Packet format: [Opcode:2][UsernameLen:1][Username:N][PasswordLen:1][Password:M][EmailLen:1][Email:K]
    if (packet.size() < sizeof(AuthOpcode) + 3) {
        auto response = create_register_response(RegisterResult::InvalidUsername);
        conn->send(response);
        return;
    }
    
    size_t offset = sizeof(AuthOpcode);
    uint8_t username_len = packet[offset++];
    
    if (offset + username_len > packet.size()) {
        auto response = create_register_response(RegisterResult::InvalidUsername);
        conn->send(response);
        return;
    }
    
    std::string username(packet.data() + offset, username_len);
    offset += username_len;
    
    uint8_t password_len = packet[offset++];
    
    if (offset + password_len > packet.size()) {
        auto response = create_register_response(RegisterResult::InvalidPassword);
        conn->send(response);
        return;
    }
    
    std::string password(packet.data() + offset, password_len);
    offset += password_len;
    
    uint8_t email_len = packet[offset++];
    std::string email;
    if (email_len > 0) {
        if (offset + email_len > packet.size()) {
            auto response = create_register_response(RegisterResult::EmailInvalid);
            conn->send(response);
            return;
        }
        email = std::string(packet.data() + offset, email_len);
    }
    
    LOG_INFO("Registration attempt: user={}, email={}", username, email);
    
    // Validate username (alphanumeric, 3-20 chars)
    static const std::regex username_regex("^[a-zA-Z0-9_]{3,20}$");
    if (!std::regex_match(username, username_regex)) {
        auto response = create_register_response(RegisterResult::InvalidUsername);
        conn->send(response);
        return;
    }
    
    // Validate password (min 6 chars)
    if (password.length() < 6) {
        auto response = create_register_response(RegisterResult::InvalidPassword);
        conn->send(response);
        return;
    }
    
    // Validate email if provided
    if (!email.empty()) {
        static const std::regex email_regex("^[^@]+@[^@]+\\.[^@]+$");
        if (!std::regex_match(email, email_regex)) {
            auto response = create_register_response(RegisterResult::EmailInvalid);
            conn->send(response);
            return;
        }
    }
    
    // Process registration
    auto result = process_registration({username, password, email, conn->get_endpoint()});
    
    if (result == RegisterResult::Success) {
        m_stats.total_registrations.fetch_add(1, std::memory_order_relaxed);
        LOG_INFO("Registration successful: user={}", username);
    } else {
        LOG_WARN("Registration failed: user={}, reason={}", 
                 username, static_cast<int>(result));
    }
    
    auto response = create_register_response(result);
    conn->send(response);
}

void AuthServer::handle_session_validate(ConnectionPtr conn, const Packet& packet) {
    // Packet format: [Opcode:2][SessionToken:36]
    if (packet.size() < sizeof(AuthOpcode) + 36) {
        conn->send(create_session_info(Session{}));
        return;
    }
    
    std::string token_str(packet.data() + sizeof(AuthOpcode), 36);
    
    try {
        auto session_id = uuids::uuid::from_string(token_str);
        
        if (validate_session(session_id)) {
            auto session_opt = get_session(session_id);
            if (session_opt) {
                LOG_DEBUG("Session validated: user={}", session_opt->username);
                conn->send(create_session_info(*session_opt));
                return;
            }
        }
    } catch (const std::exception& e) {
        LOG_WARN("Invalid session token format: {}", e.what());
    }
    
    conn->send(create_session_info(Session{}));
}

void AuthServer::handle_logout(ConnectionPtr conn, const Packet& packet) {
    // Packet format: [Opcode:2][SessionToken:36]
    if (packet.size() < sizeof(AuthOpcode) + 36) {
        return;
    }
    
    std::string token_str(packet.data() + sizeof(AuthOpcode), 36);
    
    try {
        auto session_id = uuids::uuid::from_string(token_str);
        auto session_opt = get_session(session_id);
        
        if (session_opt) {
            LOG_INFO("User logout: user={}", session_opt->username);
            remove_session(session_id);
            m_stats.active_sessions.fetch_sub(1, std::memory_order_relaxed);
        }
    } catch (const std::exception& e) {
        LOG_WARN("Invalid session token on logout: {}", e.what());
    }
}

void AuthServer::handle_session_query(ConnectionPtr conn, const Packet& packet) {
    // Inter-server session query
    // Only accept from trusted servers (would need IP whitelisting in production)
    
    if (packet.size() < sizeof(AuthOpcode) + 1) {
        return;
    }
    
    uint8_t query_type = packet[sizeof(AuthOpcode)];
    
    if (query_type == 0) { // Query by session ID
        if (packet.size() < sizeof(AuthOpcode) + 1 + 36) {
            return;
        }
        
        std::string token_str(packet.data() + sizeof(AuthOpcode) + 1, 36);
        
        try {
            auto session_id = uuids::uuid::from_string(token_str);
            auto session_opt = get_session(session_id);
            
            if (session_opt && session_opt->is_valid && !session_opt->is_expired()) {
                // Send session response
                Packet response(sizeof(AuthOpcode) + 1 + sizeof(uint32_t));
                auto* op = reinterpret_cast<AuthOpcode*>(response.data());
                *op = AuthOpcode::SessionResponse;
                response[sizeof(AuthOpcode)] = 1; // Valid
                *reinterpret_cast<uint32_t*>(response.data() + sizeof(AuthOpcode) + 1) 
                    = session_opt->user_id;
                conn->send(response);
            } else {
                Packet response(sizeof(AuthOpcode) + 1);
                auto* op = reinterpret_cast<AuthOpcode*>(response.data());
                *op = AuthOpcode::SessionResponse;
                response[sizeof(AuthOpcode)] = 0; // Invalid
                conn->send(response);
            }
        } catch (const std::exception& e) {
            LOG_WARN("Invalid session query: {}", e.what());
        }
    }
}

void AuthServer::broadcast_shutdown() {
    LOG_INFO("Broadcasting shutdown notification to all connected clients");
    
    Packet packet(sizeof(AuthOpcode));
    auto* opcode = reinterpret_cast<AuthOpcode*>(packet.data());
    *opcode = AuthOpcode::ServerShutdown;
    
    broadcast(packet);
}

auto AuthServer::get_session(const uuids::uuid& session_id) -> std::optional<Session> {
    std::shared_lock lock(m_sessions_mutex);
    
    auto it = m_sessions.find(session_id);
    if (it != m_sessions.end() && it->second.is_valid && !it->second.is_expired()) {
        return it->second;
    }
    
    return std::nullopt;
}

auto AuthServer::get_session_by_user_id(uint32_t user_id) -> std::optional<Session> {
    std::shared_lock lock(m_sessions_mutex);
    
    auto it = m_user_to_session.find(user_id);
    if (it != m_user_to_session.end()) {
        auto session_it = m_sessions.find(it->second);
        if (session_it != m_sessions.end() && 
            session_it->second.is_valid && 
            !session_it->second.is_expired()) {
            return session_it->second;
        }
    }
    
    return std::nullopt;
}

bool AuthServer::validate_session(const uuids::uuid& session_id) {
    std::shared_lock lock(m_sessions_mutex);
    
    auto it = m_sessions.find(session_id);
    return it != m_sessions.end() && it->second.is_valid && !it->second.is_expired();
}

void AuthServer::remove_session(const uuids::uuid& session_id) {
    std::unique_lock lock(m_sessions_mutex);
    
    auto it = m_sessions.find(session_id);
    if (it != m_sessions.end()) {
        m_user_to_session.erase(it->second.user_id);
        m_sessions.erase(it);
        LOG_DEBUG("Session removed: {}", uuids::to_string(session_id));
    }
}

void AuthServer::remove_all_sessions_for_user(uint32_t user_id) {
    std::unique_lock lock(m_sessions_mutex);
    
    auto it = m_user_to_session.find(user_id);
    if (it != m_user_to_session.end()) {
        m_sessions.erase(it->second);
        m_user_to_session.erase(it);
        LOG_DEBUG("All sessions removed for user: {}", user_id);
    }
}

auto AuthServer::process_login(const LoginRequestData& request, 
                               std::string& out_session_token) -> LoginResult {
    try {
        auto conn = m_db_pool->get_connection();
        
        // Check if account exists and get password hash
        static const std::string query = 
            "SELECT id, password_hash, is_banned, is_locked FROM users "
            "WHERE username = $1 AND is_deleted = FALSE";
        
        auto result = conn->exec_params(query, {request.username});
        
        if (result.rows().empty()) {
            return LoginResult::InvalidCredentials;
        }
        
        auto row = result.rows()[0];
        uint32_t user_id = row[0].as<uint32_t>();
        std::string password_hash = row[1].as<std::string>();
        bool is_banned = row[2].as<bool>();
        bool is_locked = row[3].as<bool>();
        
        if (is_banned) {
            return LoginResult::AccountBanned;
        }
        
        if (is_locked) {
            return LoginResult::AccountLocked;
        }
        
        // Verify password
        if (!verify_password(request.password, password_hash)) {
            return LoginResult::InvalidCredentials;
        }
        
        // Check if already logged in
        if (get_session_by_user_id(user_id).has_value()) {
            return LoginResult::AlreadyLoggedIn;
        }
        
        // Generate session token
        uuids::random_device rd;
        uuids::uuid_generator gen(rd);
        auto session_id = gen();
        
        // Create session
        Session session(session_id, user_id, request.username, "unknown");
        
        // Store in database
        static const std::string insert_session = 
            "INSERT INTO sessions (user_id, session_token, ip_address, expires_at) "
            "VALUES ($1, $2, $3, $4)";
        
        auto expires_at = std::chrono::system_clock::to_time_t(session.expires_at);
        conn->exec_params(insert_session, {
            user_id,
            uuids::to_string(session_id),
            request.ip_address,
            expires_at
        });
        
        // Update last login
        static const std::string update_login = 
            "UPDATE users SET last_login = NOW(), last_ip = $1 WHERE id = $2";
        conn->exec_params(update_login, {request.ip_address, user_id});
        
        // Store in memory
        {
            std::unique_lock lock(m_sessions_mutex);
            m_sessions[session_id] = session;
            m_user_to_session[user_id] = session_id;
        }
        
        out_session_token = uuids::to_string(session_id);
        return LoginResult::Success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Database error during login: {}", e.what());
        return LoginResult::DatabaseError;
    }
}

auto AuthServer::process_registration(const RegisterRequestData& request) -> RegisterResult {
    try {
        auto conn = m_db_pool->get_connection();
        
        // Check if username already exists
        static const std::string check_query = 
            "SELECT id FROM users WHERE username = $1";
        
        auto result = conn->exec_params(check_query, {request.username});
        
        if (!result.rows().empty()) {
            return RegisterResult::UsernameTaken;
        }
        
        // Hash password
        auto password_hash = hash_password(request.password);
        
        // Insert new user
        static const std::string insert_query = 
            "INSERT INTO users (username, password_hash, email, created_at, last_ip) "
            "VALUES ($1, $2, $3, NOW(), $4) RETURNING id";
        
        result = conn->exec_params(insert_query, {
            request.username,
            password_hash,
            request.email.empty() ? std::optional<std::string>{} : request.email,
            request.ip_address
        });
        
        if (result.rows().empty()) {
            return RegisterResult::DatabaseError;
        }
        
        LOG_INFO("New user registered: id={}, username={}", 
                 result.rows()[0][0].as<uint32_t>(), request.username);
        
        return RegisterResult::Success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Database error during registration: {}", e.what());
        return RegisterResult::DatabaseError;
    }
}

std::string AuthServer::hash_password(const std::string& password) {
    // Use BCrypt with cost factor 12
    char hash[Bcrypt::HASH_SIZE];
    bcrypt_gensalt(12, hash);
    bcrypt_hashpw(password.c_str(), hash, hash);
    return std::string(hash);
}

bool AuthServer::verify_password(const std::string& password, const std::string& hash) {
    char ret_hash[Bcrypt::HASH_SIZE];
    bcrypt_hashpw(password.c_str(), hash.c_str(), ret_hash);
    return std::string(ret_hash) == hash;
}

bool AuthServer::is_ip_blocked(const std::string& ip) {
    try {
        auto conn = m_db_pool->get_connection();
        
        static const std::string query = 
            "SELECT COUNT(*) FROM ip_bans "
            "WHERE ip_address = $1 OR ip_prefix = LEFT($1, LENGTH(ip_prefix)) "
            "AND (expires_at IS NULL OR expires_at > NOW())";
        
        auto result = conn->exec_params(query, {ip});
        
        if (!result.rows().empty()) {
            return result.rows()[0][0].as<int>() > 0;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error checking IP ban: {}", e.what());
    }
    
    return false;
}

void AuthServer::schedule_cleanup() {
    m_cleanup_timer.expires_after(std::chrono::minutes(5));
    m_cleanup_timer.async_wait([this](const asio::error_code& ec) {
        if (ec) {
            return;
        }
        cleanup_expired_sessions();
        schedule_cleanup();
    });
}

void AuthServer::cleanup_expired_sessions() {
    std::unique_lock lock(m_sessions_mutex);
    
    auto now = std::chrono::system_clock::now();
    auto it = m_sessions.begin();
    
    while (it != m_sessions.end()) {
        if (it->second.is_expired()) {
            m_user_to_session.erase(it->second.user_id);
            it = m_sessions.erase(it);
            m_stats.active_sessions.fetch_sub(1, std::memory_order_relaxed);
        } else {
            ++it;
        }
    }
    
    LOG_DEBUG("Cleaned up expired sessions, active: {}", m_sessions.size());
}

auto AuthServer::create_login_response(LoginResult result, const std::string& session_token) -> Packet {
    Packet packet(sizeof(AuthOpcode) + 1 + (result == LoginResult::Success ? 36 : 0));
    auto* opcode = reinterpret_cast<AuthOpcode*>(packet.data());
    *opcode = AuthOpcode::LoginResponse;
    packet[sizeof(AuthOpcode)] = static_cast<uint8_t>(result);
    
    if (result == LoginResult::Success && !session_token.empty()) {
        std::memcpy(packet.data() + sizeof(AuthOpcode) + 1, 
                    session_token.c_str(), 36);
    }
    
    return packet;
}

auto AuthServer::create_register_response(RegisterResult result) -> Packet {
    Packet packet(sizeof(AuthOpcode) + 1);
    auto* opcode = reinterpret_cast<AuthOpcode*>(packet.data());
    *opcode = AuthOpcode::RegisterResponse;
    packet[sizeof(AuthOpcode)] = static_cast<uint8_t>(result);
    return packet;
}

auto AuthServer::create_session_info(const Session& session) -> Packet {
    Packet packet(sizeof(AuthOpcode) + 1 + (session.is_valid ? 36 + 4 : 0));
    auto* opcode = reinterpret_cast<AuthOpcode*>(packet.data());
    *opcode = AuthOpcode::SessionInfo;
    packet[sizeof(AuthOpcode)] = session.is_valid ? 1 : 0;
    
    if (session.is_valid) {
        std::memcpy(packet.data() + sizeof(AuthOpcode) + 1,
                    uuids::to_string(session.session_id).c_str(), 36);
        *reinterpret_cast<uint32_t*>(packet.data() + sizeof(AuthOpcode) + 1 + 36)
            = session.user_id;
    }
    
    return packet;
}

} // namespace kal::auth
