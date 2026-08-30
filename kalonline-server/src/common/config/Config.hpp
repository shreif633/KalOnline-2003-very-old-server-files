#pragma once

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <string>
#include <mutex>
#include <expected>

namespace kal::config {

class Config {
public:
    struct DatabaseConfig {
        std::string host;
        uint16_t port;
        std::string database;
        std::string username;
        std::string password;
        int max_connections;
        int connection_timeout_sec;
        
        std::string connection_string() const;
    };
    
    struct NetworkConfig {
        std::string bind_address;
        uint16_t auth_port;
        uint16_t db_port;
        uint16_t main_port;
        int max_connections;
        int backlog;
        bool reuse_address;
        std::chrono::milliseconds read_timeout;
        std::chrono::milliseconds write_timeout;
    };
    
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
    
    struct SecurityConfig {
        bool enable_ip_block;
        uint32_t max_login_attempts;
        std::string encryption_key;
        bool enable_encryption;
        std::chrono::minutes login_block_duration;
        bool enable_ip_blocking;
        int max_requests_per_minute;
        std::vector<std::string> admin_ips;
    };
    
    struct LogConfig {
        std::string level;
        std::string directory;
        bool console_output;
        bool file_output;
        bool rotate_daily;
        int max_files;
    };
    
    static Config& instance();
    
    bool load(const std::filesystem::path& filepath);
    bool save(const std::filesystem::path& filepath = {}) const;
    
    std::string get_string(std::string_view key, std::string_view default_value = "") const;
    int get_int(std::string_view key, int default_value = 0) const;
    double get_double(std::string_view key, double default_value = 0.0) const;
    bool get_bool(std::string_view key, bool default_value = false) const;
    
    DatabaseConfig database() const;
    NetworkConfig network() const;
    GameConfig game() const;
    SecurityConfig security() const;
    LogConfig logging() const;
    
    bool has(const std::string& key) const {
        return m_data[key].IsDefined();
    }
    
    bool is_loaded() const { return m_loaded; }

private:
    Config() = default;
    
    YAML::Node m_data;
    std::filesystem::path m_filepath;
    bool m_loaded = false;
    mutable std::mutex m_mutex;
};

} // namespace kal::config
