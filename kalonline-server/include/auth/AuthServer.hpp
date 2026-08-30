#pragma once
#include "common/network/Network.hpp"
#include "common/database/Database.hpp"
#include <bcrypt/bcrypt.h>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <uuid.h>

namespace kal::auth {

// Packet opcodes (original 2003 protocol)
enum class AuthOpcode : uint16_t {
    // Client → Server
    LoginRequest = 0x1001,
    RegisterRequest = 0x1002,
    SessionValidate = 0x1003,
    LogoutRequest = 0x1004,
    
    // Server → Client
    LoginResponse = 0x2001,
    RegisterResponse = 0x2002,
    SessionInfo = 0x2003,
    KickNotification = 0x2004,
    
    // Inter-server
    SessionQuery = 0x3001,
    SessionResponse = 0x3002,
    AccountStatus = 0x3003,
    ServerShutdown = 0x3004
};

// Result codes
enum class LoginResult : uint8_t {
    Success = 0,
    InvalidCredentials = 1,
    AccountBanned = 2,
    AccountLocked = 3,
    AlreadyLoggedIn = 4,
    ServerFull = 5,
    MaintenanceMode = 6,
    DatabaseError = 7
};

enum class RegisterResult : uint8_t {
    Success = 0,
    UsernameTaken = 1,
    InvalidUsername = 2,
    InvalidPassword = 3,
    EmailInvalid = 4,
    DatabaseError = 5
};

// Session data structure
struct Session {
    uuids::uuid session_id;
    uint32_t user_id;
    std::string username;
    std::string ip_address;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point expires_at;
    bool is_valid;
    
    Session() : user_id(0), is_valid(false) {}
    
    Session(uuids::uuid sid, uint32_t uid, std::string uname, std::string ip)
        : session_id(sid), user_id(uid), username(std::move(uname)), 
          ip_address(std::move(ip)), is_valid(true) {
        created_at = std::chrono::system_clock::now();
        expires_at = created_at + std::chrono::hours(24); // 24 hour sessions
    }
    
    bool is_expired() const {
        return std::chrono::system_clock::now() > expires_at;
    }
};

// Login request packet structure
struct LoginRequestData {
    std::string username;
    std::string password;
    std::string client_version;
    uint32_t client_build;
};

// Register request packet structure  
struct RegisterRequestData {
    std::string username;
    std::string password;
    std::string email;
    std::string ip_address;
};

class AuthServer : public network::TcpServer {
public:
    explicit AuthServer(asio::io_context& io_context, uint16_t port, 
                       std::shared_ptr<database::DatabasePool> db_pool);
    
    ~AuthServer();
    
    // Start the server
    void start();
    
    // Stop the server gracefully
    void stop();
    
    // Handle incoming packets
    void on_packet_received(network::ConnectionPtr conn, const network::Packet& packet);
    
    // Authentication handlers
    void handle_login(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_register(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_session_validate(network::ConnectionPtr conn, const network::Packet& packet);
    void handle_logout(network::ConnectionPtr conn, const network::Packet& packet);
    
    // Session management
    std::optional<Session> get_session(const uuids::uuid& session_id);
    std::optional<Session> get_session_by_user_id(uint32_t user_id);
    bool validate_session(const uuids::uuid& session_id);
    void remove_session(const uuids::uuid& session_id);
    void remove_all_sessions_for_user(uint32_t user_id);
    
    // Inter-server communication
    void handle_session_query(network::ConnectionPtr conn, const network::Packet& packet);
    void broadcast_shutdown();
    
    // Statistics
    struct Stats {
        std::atomic<uint32_t> total_logins{0};
        std::atomic<uint32_t> failed_logins{0};
        std::atomic<uint32_t> total_registrations{0};
        std::atomic<uint32_t> active_sessions{0};
        std::chrono::system_clock::time_point start_time;
    };
    
    const Stats& get_stats() const { return m_stats; }
    
private:
    // Internal methods
    LoginResult process_login(const LoginRequestData& request, std::string& out_session_token);
    RegisterResult process_registration(const RegisterRequestData& request);
    std::string hash_password(const std::string& password);
    bool verify_password(const std::string& password, const std::string& hash);
    bool is_ip_blocked(const std::string& ip);
    void cleanup_expired_sessions();
    
    // Packet helpers
    network::Packet create_login_response(LoginResult result, const std::string& session_token = "");
    network::Packet create_register_response(RegisterResult result);
    network::Packet create_session_info(const Session& session);
    
    // Dependencies
    std::shared_ptr<database::DatabasePool> m_db_pool;
    
    // Session storage
    std::unordered_map<uuids::uuid, Session, uuids::UuidHash> m_sessions;
    std::unordered_map<uint32_t, uuids::uuid> m_user_to_session;
    mutable std::shared_mutex m_sessions_mutex;
    
    // Rate limiting
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_login_attempts;
    std::shared_mutex m_rate_limit_mutex;
    
    // Statistics
    Stats m_stats;
    
    // Cleanup timer
    asio::steady_timer m_cleanup_timer;
    
    // Server state
    std::atomic<bool> m_running{false};
    static constexpr int MAX_LOGIN_ATTEMPTS = 5;
    static constexpr std::chrono::minutes LOGIN_WINDOW{5};
};

} // namespace kal::auth
