#include "common/config/Config.hpp"
#include <iostream>

namespace kal::config {

Config& Config::instance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::filesystem::path& filepath) {
    try {
        m_data = YAML::LoadFile(filepath.string());
        m_filepath = filepath;
        m_loaded = true;
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "Failed to load config file " << filepath << ": " << e.what() << std::endl;
        return false;
    }
}

bool Config::save(const std::filesystem::path& filepath) const {
    if (!m_loaded) {
        return false;
    }
    
    try {
        auto save_path = filepath.empty() ? m_filepath : filepath;
        
        // Ensure directory exists
        if (auto parent = save_path.parent_path(); !parent.empty()) {
            std::filesystem::create_directories(parent);
        }
        
        YAML::Emitter out;
        out << m_data;
        
        std::ofstream fout(save_path);
        if (!fout.is_open()) {
            return false;
        }
        
        fout << out.c_str();
        return fout.good();
    } catch (const YAML::Exception&) {
        return false;
    }
}

std::string Config::get_string(std::string_view key, std::string_view default_value) const {
    try {
        auto value = m_data[key.data()];
        if (value.IsDefined()) {
            return value.as<std::string>();
        }
    } catch (...) {}
    return std::string(default_value);
}

int Config::get_int(std::string_view key, int default_value) const {
    try {
        auto value = m_data[key.data()];
        if (value.IsDefined()) {
            return value.as<int>();
        }
    } catch (...) {}
    return default_value;
}

double Config::get_double(std::string_view key, double default_value) const {
    try {
        auto value = m_data[key.data()];
        if (value.IsDefined()) {
            return value.as<double>();
        }
    } catch (...) {}
    return default_value;
}

bool Config::get_bool(std::string_view key, bool default_value) const {
    try {
        auto value = m_data[key.data()];
        if (value.IsDefined()) {
            return value.as<bool>();
        }
    } catch (...) {}
    return default_value;
}

Config::DatabaseConfig Config::database() const {
    DatabaseConfig cfg;
    const auto& db = m_data["database"];
    
    cfg.host = db["host"].as<std::string>("localhost");
    cfg.port = db["port"].as<int>(5432);
    cfg.database = db["name"].as<std::string>("kalonline");
    cfg.username = db["username"].as<std::string>("kaluser");
    cfg.password = db["password"].as<std::string>("");
    cfg.max_connections = db["max_connections"].as<int>(20);
    cfg.connection_timeout_sec = db["connection_timeout_sec"].as<int>(10);
    
    return cfg;
}

std::string Config::DatabaseConfig::connection_string() const {
    return fmt::format(
        "dbname={} user={} password={} host={} port={}",
        database, username, password, host, port
    );
}

Config::NetworkConfig Config::network() const {
    NetworkConfig cfg;
    const auto& net = m_data["network"];
    
    cfg.bind_address = net["bind_address"].as<std::string>("0.0.0.0");
    cfg.auth_port = net["auth_port"].as<int>(11000);
    cfg.db_port = net["db_port"].as<int>(11001);
    cfg.main_port = net["main_port"].as<int>(11002);
    cfg.max_connections = net["max_connections"].as<int>(2000);
    cfg.backlog = net["backlog"].as<int>(128);
    cfg.reuse_address = net["reuse_address"].as<bool>(true);
    cfg.read_timeout = std::chrono::milliseconds(net["read_timeout_ms"].as<int>(30000));
    cfg.write_timeout = std::chrono::milliseconds(net["write_timeout_ms"].as<int>(10000));
    
    return cfg;
}

Config::GameConfig Config::game() const {
    GameConfig cfg;
    const auto& game = m_data["game"];
    
    cfg.max_players = game["max_players"].as<int>(2000);
    cfg.tick_rate_ms = game["tick_rate_ms"].as<int>(50);  // 20 ticks per second
    cfg.view_distance = game["view_distance"].as<int>(20);
    cfg.exp_multiplier = game["exp_multiplier"].as<float>(1.0f);
    cfg.drop_rate_multiplier = game["drop_rate_multiplier"].as<float>(1.0f);
    cfg.gold_multiplier = game["gold_multiplier"].as<float>(1.0f);
    cfg.pvp_enabled = game["pvp_enabled"].as<bool>(true);
    cfg.trade_enabled = game["trade_enabled"].as<bool>(true);
    cfg.warehouse_enabled = game["warehouse_enabled"].as<bool>(true);
    cfg.party_max_members = game["party_max_members"].as<int>(4);
    cfg.guild_min_members = game["guild_min_members"].as<int>(3);
    cfg.guild_max_members = game["guild_max_members"].as<int>(50);
    
    return cfg;
}

Config::SecurityConfig Config::security() const {
    SecurityConfig cfg;
    const auto& sec = m_data["security"];
    
    cfg.enable_encryption = sec["enable_encryption"].as<bool>(true);
    cfg.encryption_key = sec["encryption_key"].as<std::string>("KalOnline2003Key");
    cfg.max_login_attempts = sec["max_login_attempts"].as<int>(5);
    cfg.login_block_duration = std::chrono::minutes(sec["login_block_duration_min"].as<int>(15));
    cfg.enable_ip_blocking = sec["enable_ip_blocking"].as<bool>(true);
    cfg.max_requests_per_minute = sec["max_requests_per_minute"].as<int>(100);
    
    if (sec["admin_ips"] && sec["admin_ips"].IsSequence()) {
        for (const auto& ip : sec["admin_ips"]) {
            cfg.admin_ips.push_back(ip.as<std::string>());
        }
    }
    
    return cfg;
}

Config::LogConfig Config::logging() const {
    LogConfig cfg;
    const auto& log = m_data["logging"];
    
    cfg.level = log["level"].as<std::string>("info");
    cfg.directory = log["directory"].as<std::string>("logs");
    cfg.console_output = log["console_output"].as<bool>(true);
    cfg.file_output = log["file_output"].as<bool>(true);
    cfg.rotate_daily = log["rotate_daily"].as<bool>(true);
    cfg.max_files = log["max_files"].as<int>(30);
    
    return cfg;
}

} // namespace kal::config
