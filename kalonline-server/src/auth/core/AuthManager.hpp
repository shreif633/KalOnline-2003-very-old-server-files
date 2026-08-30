#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include <atomic>

namespace kal::auth {

struct UserInfo {
    uint32_t userId;
    std::string username;
    std::string passwordHash;
    bool isLocked;
    std::string lastLoginIp;
    std::chrono::system_clock::time_point lastLoginTime;
};

struct SessionInfo {
    std::string sessionToken;
    uint32_t userId;
    std::string username;
    std::chrono::steady_clock::time_point createTime;
    std::chrono::steady_clock::time_point lastActivity;
    std::string ipAddress;
};

class AuthManager {
public:
    enum class LoginResult : uint8_t {
        Success = 0,
        InvalidCredentials = 1,
        AccountLocked = 2,
        AlreadyLoggedIn = 3,
        ServerFull = 4,
        Maintenance = 5,
        DatabaseError = 6
    };

    enum class RegisterResult : uint8_t {
        Success = 0,
        AccountExists = 1,
        InvalidUsername = 2,
        InvalidPassword = 3,
        ServerError = 4
    };

    AuthManager() = default;
    ~AuthManager() = default;

    // Authentication
    LoginResult Authenticate(
        const std::string& username,
        const std::string& password,
        const std::string& ip,
        std::string& outToken,
        uint32_t& outUserId);

    RegisterResult Register(
        const std::string& username,
        const std::string& password,
        const std::string& ip);

    // Session management
    bool ValidateSession(const std::string& token);
    void InvalidateSession(const std::string& token);
    void UpdateSessionActivity(const std::string& token);
    
    // IP blocking
    bool IsIpBlocked(const std::string& ip) const;
    void BlockIp(const std::string& ip, int durationMinutes = -1); // -1 = permanent
    void UnblockIp(const std::string& ip);

    // Statistics
    size_t GetActiveSessionCount() const;
    size_t GetBlockedIpCount() const;

private:
    std::string GenerateSessionToken();
    bool ValidateUsername(const std::string& username) const;
    bool ValidatePassword(const std::string& password) const;
    std::string HashPassword(const std::string& password) const;
    void RecordFailedAttempt(const std::string& ip);

    mutable std::mutex mutex_;
    std::unordered_map<std::string, SessionInfo> sessions_; // token -> session
    std::unordered_set<std::string> blockedIps_;
    
    static constexpr size_t MAX_ACTIVE_SESSIONS = 10000;
    static constexpr size_t TOKEN_LENGTH = 32;
};

} // namespace kal::auth
