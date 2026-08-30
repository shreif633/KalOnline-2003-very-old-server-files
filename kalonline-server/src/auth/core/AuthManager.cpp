#include "AuthManager.hpp"
#include "common/logger/Logger.hpp"
#include "common/crypto/Crypto.hpp"
#include "common/database/Database.hpp"
#include <random>
#include <algorithm>

namespace kal::auth {

AuthManager::LoginResult AuthManager::Authenticate(
    const std::string& username, 
    const std::string& password,
    const std::string& ip, 
    std::string& outToken, 
    uint32_t& outUserId) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if IP is blocked
    if (IsIpBlocked(ip)) {
        KAL_LOG_WARNING("Login attempt from blocked IP: {}", ip);
        return LoginResult::AccountLocked;
    }
    
    try {
        auto db = kal::database::Database::getInstance();
        
        // Query user from database
        auto result = db->executeQuery(
            "SELECT id, password_hash, is_locked FROM users WHERE username = $1",
            {username}
        );
        
        if (!result || !result->Next()) {
            KAL_LOG_WARNING("Login failed: User {} not found", username);
            RecordFailedAttempt(ip);
            return LoginResult::InvalidCredentials;
        }
        
        outUserId = static_cast<uint32_t>(result->GetInt(0));
        std::string storedHash = result->GetString(1);
        bool isLocked = (result->GetString(2) == "t");
        
        if (isLocked) {
            KAL_LOG_WARNING("Login failed: Account {} is locked", username);
            return LoginResult::AccountLocked;
        }
        
        // Verify password hash
        std::string inputHash = HashPassword(password);
        if (inputHash != storedHash) {
            KAL_LOG_WARNING("Login failed: Invalid password for user {}", username);
            RecordFailedAttempt(ip);
            return LoginResult::InvalidCredentials;
        }
        
        // Generate session token
        outToken = GenerateSessionToken();
        
        // Store session
        SessionInfo session;
        session.sessionToken = outToken;
        session.userId = outUserId;
        session.username = username;
        session.createTime = std::chrono::steady_clock::now();
        session.lastActivity = session.createTime;
        session.ipAddress = ip;
        
        sessions_[outToken] = session;
        
        KAL_LOG_INFO("User {} logged in successfully (ID: {})", username, outUserId);
        return LoginResult::Success;
        
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Database error during authentication: {}", e.what());
        RecordFailedAttempt(ip);
        return LoginResult::DatabaseError;
    }
}

AuthManager::RegisterResult AuthManager::Register(
    const std::string& username,
    const std::string& password,
    const std::string& ip)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!ValidateUsername(username)) {
        return RegisterResult::InvalidUsername;
    }
    
    if (!ValidatePassword(password)) {
        return RegisterResult::InvalidPassword;
    }
    
    try {
        auto db = kal::database::Database::getInstance();
        
        // Check if user exists
        auto checkResult = db->executeQuery(
            "SELECT id FROM users WHERE username = $1",
            {username}
        );
        
        if (checkResult && checkResult->Next()) {
            return RegisterResult::AccountExists;
        }
        
        // Insert new user
        std::string passwordHash = HashPassword(password);
        db->executeCommand(
            "INSERT INTO users (username, password_hash, created_at) VALUES ($1, $2, NOW())",
            {username, passwordHash}
        );
        
        KAL_LOG_INFO("New user registered: {}", username);
        return RegisterResult::Success;
        
    } catch (const std::exception& e) {
        KAL_LOG_ERROR("Database error during registration: {}", e.what());
        return RegisterResult::ServerError;
    }
}

bool AuthManager::ValidateSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it == sessions_.end()) {
        return false;
    }
    
    // Check if session expired (30 minutes)
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
        now - it->second.lastActivity);
    
    if (elapsed.count() > 30) {
        sessions_.erase(it);
        return false;
    }
    
    return true;
}

void AuthManager::InvalidateSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(token);
}

void AuthManager::UpdateSessionActivity(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it != sessions_.end()) {
        it->second.lastActivity = std::chrono::steady_clock::now();
    }
}

bool AuthManager::IsIpBlocked(const std::string& ip) const {
    return blockedIps_.find(ip) != blockedIps_.end();
}

void AuthManager::BlockIp(const std::string& ip, int durationMinutes) {
    std::lock_guard<std::mutex> lock(mutex_);
    blockedIps_.insert(ip);
    KAL_LOG_WARNING("IP blocked: {}", ip);
}

void AuthManager::UnblockIp(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mutex_);
    blockedIps_.erase(ip);
}

size_t AuthManager::GetActiveSessionCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.size();
}

size_t AuthManager::GetBlockedIpCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return blockedIps_.size();
}

std::string AuthManager::GenerateSessionToken() {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 61);
    
    std::string token;
    token.reserve(32);
    for (int i = 0; i < 32; ++i) {
        token += chars[dis(gen)];
    }
    return token;
}

bool AuthManager::ValidateUsername(const std::string& username) const {
    if (username.length() < 4 || username.length() > 20) {
        return false;
    }
    for (char c : username) {
        if (!std::isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

bool AuthManager::ValidatePassword(const std::string& password) const {
    return password.length() >= 6;
}

std::string AuthManager::HashPassword(const std::string& password) const {
    return kal::crypto::SHA256(password);
}

void AuthManager::RecordFailedAttempt(const std::string& ip) {
    // Simple implementation - could be enhanced with time-based tracking
    KAL_LOG_DEBUG("Failed login attempt from IP: {}", ip);
}

} // namespace kal::auth
