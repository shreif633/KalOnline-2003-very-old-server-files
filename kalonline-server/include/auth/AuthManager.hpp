#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include "Database.hpp"

struct UserSession {
    uint32_t userId;
    std::string username;
    std::string sessionToken;
    std::string ipAddress;
    std::chrono::steady_clock::time_point loginTime;
    bool isActive;
};

class AuthManager {
private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<UserSession>> sessions_; // token -> session
    std::unordered_map<uint32_t, std::string> userToToken_; // userId -> token
    
    static const int SESSION_TIMEOUT_MINUTES = 30;
    static const int MAX_FAILED_ATTEMPTS = 5;
    
    struct FailedAttempt {
        std::string ip;
        std::chrono::steady_clock::time_point time;
        int count;
    };
    std::unordered_map<std::string, FailedAttempt> failedAttempts_;

public:
    AuthManager() = default;
    ~AuthManager() = default;

    // Authentication
    enum class LoginResult {
        Success,
        InvalidCredentials,
        AccountLocked,
        AlreadyLoggedIn,
        ServerFull,
        DatabaseError
    };
    
    LoginResult Authenticate(const std::string& username, const std::string& password, 
                            const std::string& ip, std::string& outToken, uint32_t& outUserId);
    
    bool Logout(const std::string& token);
    bool ValidateSession(const std::string& token);
    
    // Session Management
    std::shared_ptr<UserSession> GetSession(const std::string& token);
    void RemoveSession(const std::string& token);
    void CleanupExpiredSessions();
    
    // IP Blocking
    bool IsIpBlocked(const std::string& ip);
    void RecordFailedAttempt(const std::string& ip);
    void ClearFailedAttempts(const std::string& ip);
    
    // Statistics
    size_t GetActiveSessionCount() const;
    std::vector<std::shared_ptr<UserSession>> GetAllActiveSessions() const;
};
