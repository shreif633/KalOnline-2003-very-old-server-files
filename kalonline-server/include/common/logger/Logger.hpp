#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <source_location>
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/chrono.h>

namespace kal::logger {

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

[[nodiscard]] constexpr std::string_view level_to_string(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:                return "UNKN ";
    }
}

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
    std::string source_file;
    int source_line;
    std::string function;
    std::thread::id thread_id;
};

class LoggerBackend {
public:
    virtual ~LoggerBackend() = default;
    virtual void write(const LogEntry& entry) = 0;
};

class ConsoleBackend : public LoggerBackend {
public:
    void write(const LogEntry& entry) override;
};

class FileBackend : public LoggerBackend {
public:
    explicit FileBackend(const std::filesystem::path& filepath);
    void write(const LogEntry& entry) override;
    
private:
    std::ofstream m_file;
    std::mutex m_mutex;
};

class AsyncLogger {
public:
    using clock = std::chrono::steady_clock;
    
    static AsyncLogger& instance();
    
    void initialize(std::string_view app_name, LogLevel min_level = LogLevel::Info);
    void shutdown();
    
    void log(LogLevel level, std::string_view message,
             const std::source_location& location = std::source_location::current());
    
    template<typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        log_formatted(LogLevel::Trace, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log_formatted(LogLevel::Debug, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log_formatted(LogLevel::Info, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warning(fmt::format_string<Args...> fmt, Args&&... args) {
        log_formatted(LogLevel::Warning, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log_formatted(LogLevel::Error, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void fatal(fmt::format_string<Args...> fmt, Args&&... args) {
        log_formatted(LogLevel::Fatal, fmt, std::forward<Args>(args)...);
    }
    
    void set_min_level(LogLevel level) noexcept { m_min_level = level; }
    [[nodiscard]] LogLevel min_level() const noexcept { return m_min_level; }
    
    void add_backend(std::unique_ptr<LoggerBackend> backend);
    
private:
    template<typename... Args>
    void log_formatted(LogLevel level, fmt::format_string<Args...> fmt, Args&&... args) {
        if (level >= m_min_level) {
            log(level, fmt::format(fmt, std::forward<Args>(args)...));
        }
    }
    
    void worker_thread_func();
    [[nodiscard]] std::string format_entry(const LogEntry& entry) const;
    
    std::string m_app_name;
    std::atomic<LogLevel> m_min_level{LogLevel::Info};
    std::atomic<bool> m_running{false};
    
    std::queue<LogEntry> m_queue;
    mutable std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    
    std::vector<std::unique_ptr<LoggerBackend>> m_backends;
    std::thread m_worker_thread;
    
    static constexpr size_t MAX_QUEUE_SIZE = 4096;
};

// Convenience macros
#define KAL_LOG_TRACE(...) ::kal::logger::AsyncLogger::instance().trace(__VA_ARGS__)
#define KAL_LOG_DEBUG(...) ::kal::logger::AsyncLogger::instance().debug(__VA_ARGS__)
#define KAL_LOG_INFO(...)  ::kal::logger::AsyncLogger::instance().info(__VA_ARGS__)
#define KAL_LOG_WARNING(...) ::kal::logger::AsyncLogger::instance().warning(__VA_ARGS__)
#define KAL_LOG_ERROR(...) ::kal::logger::AsyncLogger::instance().error(__VA_ARGS__)
#define KAL_LOG_FATAL(...) ::kal::logger::AsyncLogger::instance().fatal(__VA_ARGS__)

} // namespace kal::logger
