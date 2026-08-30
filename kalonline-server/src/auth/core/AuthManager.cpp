#include "AuthManager.hpp"
#include "Database.hpp"
#include "Crypto.hpp"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>

#ifdef HAVE_BCRYPT
#include <bcrypt/bcrypt.h>
#endif

namespace kal::auth {

bool UserSession::is_valid() const {
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return expires_at > now;
}

AuthManager& AuthManager::instance() {
    static AuthManager instance;
    return instance;
}

std::expected<LoginResult, std::string> AuthManager::login(
    const std::string& username,
    const std::string& password,
    const std::string& ip_address) {
    
    // Check IP block
    if (is_ip_blocked(ip_address)) {
        return std::unexpected("IP address is blocked");
    }
    
    try {
        auto& db = kal::Database::instance();
        
        // Check if account exists and get password hash
        std::string query = R"(
            SELECT id, password_hash, is_locked, is_deleted
            FROM users 
            WHERE username = $1
        )";
        
        auto result = db.execute_query(query, username);
        if (!result || result->empty()) {
            return LoginResult::InvalidCredentials;
        }
        
        auto row = *result->begin();
        uint32_t user_id = row.get<uint32_t>(0);
        std::string password_hash = row.get<std::string>(1);
        bool is_locked = row.get<bool>(2);
        bool is_deleted = row.get<bool>(3);
        
        if (is_deleted) {
            return LoginResult::AccountLocked;
        }
        
        if (is_locked) {
            return LoginResult::AccountLocked;
        }
        
        // Verify password
        if (!verify_password(password, password_hash)) {
            // Increment failed login counter
            std::lock_guard<std::shared_mutex> lock(stats_mutex_);
            current_stats_.failed_logins_today++;
            return LoginResult::InvalidCredentials;
        }
        
        // Check if already logged in
        std::lock_guard<std::shared_mutex> session_lock(sessions_mutex_);
        for (const auto& [token, session] : active_sessions_) {
            if (session.user_id == user_id && session.is_valid()) {
                return LoginResult::AlreadyLoggedIn;
            }
        }
        
        // Create session
        std::string session_token = generate_session_token();
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        UserSession session{
            .username = username,
            .session_token = session_token,
            .user_id = user_id,
            .created_at = now,
            .expires_at = now + 86400, // 24 hours
            .ip_address = ip_address
        };
        
        active_sessions_[session_token] = session;
        
        // Update last login in database
        db.execute_query("UPDATE users SET last_login = NOW(), last_ip = $1 WHERE id = $2",
                        ip_address, user_id);
        
        // Log successful login
        LOG_INFO("User '{}' logged in successfully (ID: {})", username, user_id);
        
        // Update statistics
        {
            std::lock_guard<std::shared_mutex> lock(stats_mutex_);
            current_stats_.total_logins_today++;
        }
        
        return LoginResult::Success;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Login failed for '{}': {}", username, e.what());
        return std::unexpected(std::format("Database error: {}", e.what()));
    }
}

std::expected<RegisterResult, std::string> AuthManager::register_account(
    const std::string& username,
    const std::string& password,
    const std::string& email) {
    
    // Validate username
    if (username.length() < 4 || username.length() > 20) {
        return RegisterResult::InvalidUsername;
    }
    
    // Validate password
    if (password.length() < 6) {
        return RegisterResult::InvalidPassword;
    }
    
    // Validate email (basic check)
    if (email.find('@') == std::string::npos) {
        return RegisterResult::InvalidEmail;
    }
    
    try {
        auto& db = kal::Database::instance();
        
        // Hash password
        auto hash_result = hash_password(password);
        if (!hash_result) {
            return std::unexpected("Failed to hash password");
        }
        
        // Insert new user
        std::string query = R"(
            INSERT INTO users (username, password_hash, email, created_at)
            VALUES ($1, $2, $3, NOW())
            RETURNING id
        )";
        
        auto result = db.execute_query(query, username, *hash_result, email);
        if (!result || result->empty()) {
            return RegisterResult::DatabaseError;
        }
        
        LOG_INFO("New user registered: '{}' (ID: {})", username, 
                 result->begin()->get<uint32_t>(0));
        
        // Update statistics
        {
            std::lock_guard<std::shared_mutex> lock(stats_mutex_);
            current_stats_.registered_accounts_today++;
        }
        
        return RegisterResult::Success;
        
    } catch (const pqxx::unique_violation& e) {
        std::string msg = e.what();
        if (msg.find("username") != std::string::npos) {
            return RegisterResult::UsernameTaken;
        } else if (msg.find("email") != std::string::npos) {
            return RegisterResult::EmailTaken;
        }
        return std::unexpected(std::format("Database error: {}", e.what()));
    } catch (const std::exception& e) {
        LOG_ERROR("Registration failed for '{}': {}", username, e.what());
        return std::unexpected(std::format("Database error: {}", e.what()));
    }
}

LogoutResult AuthManager::logout(const std::string& session_token) {
    std::lock_guard<std::shared_mutex> lock(sessions_mutex_);
    
    auto it = active_sessions_.find(session_token);
    if (it != active_sessions_.end()) {
        LOG_INFO("User '{}' logged out", it->second.username);
        active_sessions_.erase(it);
        return LogoutResult::Success;
    }
    
    return LogoutResult::InvalidSession;
}

std::expected<UserSession, std::string> AuthManager::validate_session(
    const std::string& session_token) {
    
    std::shared_lock<std::shared_mutex> lock(sessions_mutex_);
    
    auto it = active_sessions_.find(session_token);
    if (it == active_sessions_.end()) {
        return std::unexpected("Session not found");
    }
    
    if (!it->second.is_valid()) {
        return std::unexpected("Session expired");
    }
    
    return it->second;
}

void AuthManager::invalidate_session(const std::string& session_token) {
    std::lock_guard<std::shared_mutex> lock(sessions_mutex_);
    active_sessions_.erase(session_token);
}

void AuthManager::invalidate_all_sessions(uint32_t user_id) {
    std::lock_guard<std::shared_mutex> lock(sessions_mutex_);
    
    for (auto it = active_sessions_.begin(); it != active_sessions_.end();) {
        if (it->second.user_id == user_id) {
            it = active_sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

bool AuthManager::is_ip_blocked(const std::string& ip_address) {
    std::shared_lock<std::shared_mutex> lock(blocked_ips_mutex_);
    
    auto it = blocked_ips_.find(ip_address);
    if (it == blocked_ips_.end()) {
        return false;
    }
    
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (it->second > now) {
        return true;
    }
    
    // Block expired, remove it
    lock.unlock();
    std::lock_guard<std::shared_mutex> write_lock(blocked_ips_mutex_);
    blocked_ips_.erase(ip_address);
    
    return false;
}

void AuthManager::block_ip(const std::string& ip_address, uint64_t duration_seconds) {
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::lock_guard<std::shared_mutex> lock(blocked_ips_mutex_);
    blocked_ips_[ip_address] = now + duration_seconds;
    
    LOG_WARN("IP {} blocked for {} seconds", ip_address, duration_seconds);
}

void AuthManager::unblock_ip(const std::string& ip_address) {
    std::lock_guard<std::shared_mutex> lock(blocked_ips_mutex_);
    blocked_ips_.erase(ip_address);
    LOG_INFO("IP {} unblocked", ip_address);
}

AuthManager::Stats AuthManager::get_statistics() const {
    std::shared_lock<std::shared_mutex> lock(stats_mutex_);
    
    Stats stats = current_stats_;
    stats.active_sessions = 0;
    
    // Count valid sessions
    std::shared_lock<std::shared_mutex> session_lock(sessions_mutex_);
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    for (const auto& [_, session] : active_sessions_) {
        if (session.expires_at > now) {
            stats.active_sessions++;
        }
    }
    
    return stats;
}

std::expected<std::string, std::string> AuthManager::hash_password(
    const std::string& password) {
    
#ifdef HAVE_BCRYPT
    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];
    
    if (bcrypt_gensalt(12, salt) != 0) {
        return std::unexpected("Failed to generate salt");
    }
    
    if (bcrypt_hashpw(password.c_str(), salt, hash) != 0) {
        return std::unexpected("Failed to hash password");
    }
    
    return std::string(hash);
#else
    // Fallback: Use SHA-256 with salt (less secure but works without bcrypt)
    std::string salt = "kalonline_salt_v1_";
    std::string salted_password = salt + password;
    
    // Simple SHA-256 implementation would go here
    // For now, use a basic hash (NOT recommended for production)
    std::stringstream ss;
    for (char c : salted_password) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    
    LOG_WARN("Using fallback password hashing (not bcrypt)");
    return ss.str();
#endif
}

bool AuthManager::verify_password(
    const std::string& password, 
    const std::string& hash) {
    
#ifdef HAVE_BCRYPT
    return bcrypt_checkpw(password.c_str(), hash.c_str()) == 0;
#else
    // Fallback verification for non-bcrypt hashes
    std::string salt = "kalonline_salt_v1_";
    std::string salted_password = salt + password;
    
    std::stringstream ss;
    for (char c : salted_password) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    
    return ss.str() == hash;
#endif
}

std::string AuthManager::generate_session_token() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    // Generate 32-character hex token
    for (int i = 0; i < 4; i++) {
        ss << std::setw(16) << dis(gen);
    }
    
    return ss.str();
}

} // namespace kal::auth
