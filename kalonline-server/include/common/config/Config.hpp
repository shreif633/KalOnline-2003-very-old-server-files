#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <vector>
#include <map>
#include <filesystem>

#include "yaml-cpp/yaml.h"

namespace kal::config {

class ConfigError : public std::runtime_error {
public:
    explicit ConfigError(const std::string& msg) : std::runtime_error(msg) {}
};

class Config {
public:
    static Config& instance();
    
    bool load(const std::filesystem::path& filepath);
    bool save(const std::filesystem::path& filepath) const;
    
    // Template-based getters with defaults
    template<typename T>
    T get(std::string_view key, const T& default_value) const {
        try {
            auto value = m_data[key.data()];
            if (value.IsDefined()) {
                return value.as<T>();
            }
        } catch (...) {
            // Return default on any error
        }
        return default_value;
    }
    
    // Specialized getters for common types
    std::string get_string(std::string_view key, std::string_view default_value) const;
    int get_int(std::string_view key, int default_value) const;
    double get_double(std::string_view key, double default_value) const;
    bool get_bool(std::string_view key, bool default_value) const;
    
    // Database configuration
    struct DatabaseConfig {
        std::string host;
        int port;
        std::string database;
        std::string username;
        std::string password;
        int max_connections;
        int connection_timeout_sec;
        
        [[nodiscard]] std::string connection_string() const;
    };
    
    [[nodiscard]] DatabaseConfig database() const;
    
    // Network configuration
    struct NetworkConfig {
        std::string bind_address;
        int auth_port;
        int db_port;
        int main_port;
        int max_connections;
        int backlog;
        bool reuse_address;
        std::chrono::milliseconds read_timeout;
        std::chrono::milliseconds write_timeout;
    };
    
    [[nodiscard]] NetworkConfig network() const;
    
    // Game configuration
    struct GameConfig {
        int max_players;
        int tick_rate_ms;
        int view_distance;
        float exp_multiplier;
        float drop_rate_multiplier;
        float gold_multiplier;
        bool pvp_enabled;
        bool trade_enabled;
        bool warehouse_enabled;
        int party_max_members;
        int guild_min_members;
        int guild_max_members;
    };
    
    [[nodiscard]] GameConfig game() const;
    
    // Security configuration
    struct SecurityConfig {
        bool enable_encryption;
        std::string encryption_key;
        int max_login_attempts;
        std::chrono::minutes login_block_duration;
        bool enable_ip_blocking;
        int max_requests_per_minute;
        std::vector<std::string> admin_ips;
    };
    
    [[nodiscard]] SecurityConfig security() const;
    
    // Logging configuration
    struct LogConfig {
        std::string level;
        std::filesystem::path directory;
        bool console_output;
        bool file_output;
        bool rotate_daily;
        int max_files;
    };
    
    [[nodiscard]] LogConfig logging() const;
    
    // Direct YAML node access for complex structures
    const YAML::Node& raw() const noexcept { return m_data; }
    
private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    YAML::Node m_data;
    std::filesystem::path m_filepath;
    bool m_loaded{false};
};

// Convenience macros
#define KAL_CONFIG_GET(type, key, default_val) \
    ::kal::config::Config::instance().get<type>(key, default_val)

#define KAL_CONFIG_STRING(key, default_val) \
    ::kal::config::Config::instance().get_string(key, default_val)

#define KAL_CONFIG_INT(key, default_val) \
    ::kal::config::Config::instance().get_int(key, default_val)

#define KAL_CONFIG_BOOL(key, default_val) \
    ::kal::config::Config::instance().get_bool(key, default_val)

} // namespace kal::config
