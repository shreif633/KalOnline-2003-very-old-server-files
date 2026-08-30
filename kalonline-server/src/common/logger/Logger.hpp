#pragma once

#include <string>
#include <string_view>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <atomic>
#include <source_location>
#include <fstream>
#include <filesystem>
#include <vector>
#include <fmt/format.h>

namespace kal::logger {

enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
    std::string source_file;
    int source_line;
    std::string function;
    std::thread::id thread_id;
};

class IBackend {
public:
    virtual ~IBackend() = default;
    virtual void write(const LogEntry& entry) = 0;
};

class ConsoleBackend : public IBackend {
public:
    void write(const LogEntry& entry) override;
};

class FileBackend : public IBackend {
public:
    explicit FileBackend(const std::filesystem::path& filepath);
    void write(const LogEntry& entry) override;

private:
    std::ofstream m_file;
    std::mutex m_mutex;
};

class AsyncLogger {
public:
    static AsyncLogger& instance();
    
    void initialize(std::string_view app_name, LogLevel min_level = LogLevel::Info);
    void shutdown();
    
    void log(LogLevel level, std::string_view message,
             const std::source_location& location = std::source_location::current());
    
    void add_backend(std::unique_ptr<IBackend> backend);

private:
    void worker_thread_func();
    std::string format_entry(const LogEntry& entry) const;
    
    std::string m_app_name;
    std::atomic<LogLevel> m_min_level{LogLevel::Info};
    std::atomic<bool> m_running{false};
    
    std::vector<std::unique_ptr<IBackend>> m_backends;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::queue<LogEntry> m_queue;
    
    std::thread m_worker_thread;
    
    static constexpr size_t MAX_QUEUE_SIZE = 10000;
};

inline const char* level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "UNKNOWN";
    }
}

} // namespace kal::logger

#define KAL_LOG_TRACE(msg, ...) \
    kal::logger::AsyncLogger::instance().log(kal::logger::LogLevel::Trace, fmt::format(msg, ##__VA_ARGS__))

#define KAL_LOG_DEBUG(msg, ...) \
    kal::logger::AsyncLogger::instance().log(kal::logger::LogLevel::Debug, fmt::format(msg, ##__VA_ARGS__))

#define KAL_LOG_INFO(msg, ...) \
    kal::logger::AsyncLogger::instance().log(kal::logger::LogLevel::Info, fmt::format(msg, ##__VA_ARGS__))

#define KAL_LOG_WARNING(msg, ...) \
    kal::logger::AsyncLogger::instance().log(kal::logger::LogLevel::Warning, fmt::format(msg, ##__VA_ARGS__))

#define KAL_LOG_ERROR(msg, ...) \
    kal::logger::AsyncLogger::instance().log(kal::logger::LogLevel::Error, fmt::format(msg, ##__VA_ARGS__))

#define KAL_LOG_FATAL(msg, ...) \
    kal::logger::AsyncLogger::instance().log(kal::logger::LogLevel::Fatal, fmt::format(msg, ##__VA_ARGS__))
