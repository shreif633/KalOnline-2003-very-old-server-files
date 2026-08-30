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
        KAL_LOG_WARN("Login attempt from blocked IP: {}", ip);
        return LoginResult::AccountLocked;
    }
    
    try {
        auto db = kal::database::Database::GetInstance();
        
        // Query user from database
        auto result = db->ExecuteQuery(
            "SELECT id, password_hash, is_locked FROM users WHERE username = $1",
            {username}
        );
        
        if (!result || !result->Next()) {
            KAL_LOG_WARN("Login failed: User {} not found", username);
            return LoginResult::InvalidCredentials;
        }
        
        outUserId = static_cast<uint32_t>(result->GetInt(0));
        std::string storedHash = result->GetString(1);
        bool isLocked = result->GetBool(2);
        
        if (isLocked) {
            KAL_LOG_WARN("Login failed: Account {} is locked", username);
            return LoginResult::AccountLocked;
        }
        
        // Verify password hash
        std::string inputHash = HashPassword(password);
        if (inputHash != storedHash) {
            KAL_LOG_WARN("Login failed: Invalid password for user {}", username);
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
        return LoginResult::DatabaseError;
    }
}
        
        if (!result || !result->Next()) {
            RecordFailedAttempt(ip);
            return LoginResult::InvalidCredentials;
        }
        
        uint32_t userId = result->GetInt(0);
        std::string passwordHash = result->GetString(1);
        bool isLocked = result->GetBool(2);
        
        if (isLocked) {
            Logger::Warn("Login attempt on locked account: {}", username);
            return LoginResult::AccountLocked;
        }
        
        // Verify password (BCrypt)
        if (!Crypto::VerifyPassword(password, passwordHash)) {
            RecordFailedAttempt(ip);
            return LoginResult::InvalidCredentials;
        }
        
        // Check if already logged in
        auto it = userToToken_.find(userId);
        if (it != userToToken_.end() && !it->second.empty()) {
            auto existingSession = GetSession(it->second);
            if (existingSession && existingSession->isActive) {
                Logger::Info("User {} already logged in, forcing logout", username);
                Logout(it->second);
            }
        }
        
        // Generate session token
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        std::string token;
        token.reserve(32);
        for (int i = 0; i < 32; ++i) {
            token += "0123456789ABCDEF"[dis(gen)];
        }
        
        // Create session
        auto session = std::make_shared<UserSession>();
        session->userId = userId;
        session->username = username;
        session->sessionToken = token;
        session->ipAddress = ip;
        session->loginTime = std::chrono::steady_clock::now();
        session->isActive = true;
        
        sessions_[token] = session;
        userToToken_[userId] = token;
        
        // Update last login in database
        db->ExecuteQuery(
            "UPDATE users SET last_login = NOW(), last_ip = $1 WHERE id = $2",
            {ip, std::to_string(userId)}
        );
        
        outToken = token;
        outUserId = userId;
        
        ClearFailedAttempts(ip);
        Logger::Info("User {} logged in successfully (ID: {})", username, userId);
        
        return LoginResult::Success;
        
    } catch (const std::exception& e) {
        Logger::Error("Database error during authentication: {}", e.what());
        RecordFailedAttempt(ip);
        return LoginResult::DatabaseError;
    }
}

bool AuthManager::Logout(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it == sessions_.end()) {
        return false;
    }
    
    auto session = it->second;
    uint32_t userId = session->userId;
    
    // Remove from maps
    userToToken_.erase(userId);
    sessions_.erase(it);
    
    Logger::Info("User {} logged out", session->username);
    return true;
}

bool AuthManager::ValidateSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it == sessions_.end()) {
        return false;
    }
    
    auto session = it->second;
    
    // Check timeout
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - session->loginTime);
    
    if (elapsed.count() > SESSION_TIMEOUT_MINUTES) {
        Logger::Info("Session expired for user {}", session->username);
        return false;
    }
    
    return session->isActive;
}

std::shared_ptr<UserSession> AuthManager::GetSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it != sessions_.end()) {
        return it->second;
    }
    
    return nullptr;
}

void AuthManager::RemoveSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it != sessions_.end()) {
        uint32_t userId = it->second->userId;
        userToToken_.erase(userId);
        sessions_.erase(it);
    }
}

void AuthManager::CleanupExpiredSessions() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    std::vector<std::string> toRemove;
    
    for (const auto& pair : sessions_) {
        auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
            now - pair.second->loginTime);
        
        if (elapsed.count() > SESSION_TIMEOUT_MINUTES) {
            toRemove.push_back(pair.first);
        }
    }
    
    for (const auto& token : toRemove) {
        auto session = sessions_[token];
        Logger::Info("Cleaning up expired session for user {}", session->username);
        uint32_t userId = session->userId;
        userToToken_.erase(userId);
        sessions_.erase(token);
    }
}

bool AuthManager::IsIpBlocked(const std::string& ip) {
    auto it = failedAttempts_.find(ip);
    if (it == failedAttempts_.end()) {
        return false;
    }
    
    // Block for 15 minutes after max failed attempts
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - it->second.time);
    
    if (it->second.count >= MAX_FAILED_ATTEMPTS && elapsed.count() < 15) {
        return true;
    }
    
    // Reset if enough time passed
    if (elapsed.count() >= 15) {
        failedAttempts_.erase(it);
    }
    
    return false;
}

void AuthManager::RecordFailedAttempt(const std::string& ip) {
    auto it = failedAttempts_.find(ip);
    if (it == failedAttempts_.end()) {
        FailedAttempt attempt;
        attempt.ip = ip;
        attempt.time = std::chrono::steady_clock::now();
        attempt.count = 1;
        failedAttempts_[ip] = attempt;
    } else {
        it->second.count++;
        it->second.time = std::chrono::steady_clock::now();
        
        if (it->second.count >= MAX_FAILED_ATTEMPTS) {
            Logger::Warn("IP {} blocked due to multiple failed login attempts", ip);
        }
    }
}

void AuthManager::ClearFailedAttempts(const std::string& ip) {
    failedAttempts_.erase(ip);
}

size_t AuthManager::GetActiveSessionCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.size();
}

std::vector<std::shared_ptr<UserSession>> AuthManager::GetAllActiveSessions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<UserSession>> result;
    result.reserve(sessions_.size());
    
    for (const auto& pair : sessions_) {
        if (pair.second->isActive) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}
