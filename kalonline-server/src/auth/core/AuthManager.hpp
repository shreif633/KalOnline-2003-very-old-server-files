#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <shared_mutex>
#include <expected>
#include <format>
#include "Logger.hpp"

namespace kal::auth {

enum class LoginResult : uint8_t {
    Success = 0,
    InvalidCredentials = 1,
    AccountLocked = 2,
    AlreadyLoggedIn = 3,
    ServerFull = 4,
    MaintenanceMode = 5,
    IPBanned = 6,
    DatabaseError = 7,
    UnknownError = 255
};

enum class RegisterResult : uint8_t {
    Success = 0,
    UsernameTaken = 1,
    EmailTaken = 2,
    InvalidUsername = 3,
    InvalidPassword = 4,
    InvalidEmail = 5,
    DatabaseError = 6,
    UnknownError = 255
};

struct UserSession {
    std::string username;
    std::string session_token;
    uint32_t user_id;
    uint64_t created_at;
    uint64_t expires_at;
    std::string ip_address;
    bool is_valid() const;
};

class AuthManager {
public:
    static AuthManager& instance();
    
    // Authentication operations
    std::expected<LoginResult, std::string> login(
        const std::string& username,
        const std::string& password,
        const std::string& ip_address);
    
    std::expected<RegisterResult, std::string> register_account(
        const std::string& username,
        const std::string& password,
        const std::string& email);
    
    LogoutResult logout(const std::string& session_token);
    
    // Session validation
    std::expected<UserSession, std::string> validate_session(
        const std::string& session_token);
    
    void invalidate_session(const std::string& session_token);
    void invalidate_all_sessions(uint32_t user_id);
    
    // IP blocking
    bool is_ip_blocked(const std::string& ip_address);
    void block_ip(const std::string& ip_address, uint64_t duration_seconds);
    void unblock_ip(const std::string& ip_address);
    
    // Statistics
    struct Stats {
        uint32_t active_sessions;
        uint32_t total_logins_today;
        uint32_t failed_logins_today;
        uint32_t registered_accounts_today;
    };
    Stats get_statistics() const;
    
private:
    AuthManager() = default;
    ~AuthManager() = default;
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;
    
    std::expected<std::string, std::string> hash_password(
        const std::string& password);
    bool verify_password(const std::string& password, const std::string& hash);
    std::string generate_session_token();
    
    mutable std::shared_mutex sessions_mutex_;
    std::unordered_map<std::string, UserSession> active_sessions_;
    
    mutable std::shared_mutex blocked_ips_mutex_;
    std::unordered_map<std::string, uint64_t> blocked_ips_; // ip -> expiry
    
    mutable std::shared_mutex stats_mutex_;
    Stats current_stats_;
};

} // namespace kal::auth
