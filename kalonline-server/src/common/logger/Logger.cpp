#include "common/logger/Logger.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace kal::logger {

// ============================================================================
// ConsoleBackend Implementation
// ============================================================================

void ConsoleBackend::write(const LogEntry& entry) {
    static std::mutex console_mutex;
    std::lock_guard<std::mutex> lock(console_mutex);
    
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count()
        << " [" << std::setw(5) << level_to_string(entry.level) << "] "
        << "[" << std::this_thread::get_id() << "] "
        << entry.message
        << " (" << std::filesystem::path(entry.source_file).filename().string()
        << ":" << entry.source_line << ")";
    
    std::cout << oss.str() << std::endl;
}

// ============================================================================
// FileBackend Implementation
// ============================================================================

FileBackend::FileBackend(const std::filesystem::path& filepath) {
    // Ensure directory exists
    if (auto parent = filepath.parent_path(); !parent.empty()) {
        std::filesystem::create_directories(parent);
    }
    
    m_file.open(filepath, std::ios::app);
    if (!m_file.is_open()) {
        throw std::runtime_error(std::format("Failed to open log file: {}", filepath.string()));
    }
}

void FileBackend::write(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count()
        << " [" << std::setw(5) << level_to_string(entry.level) << "] "
        << "[" << std::this_thread::get_id() << "] "
        << entry.message
        << " (" << std::filesystem::path(entry.source_file).filename().string()
        << ":" << entry.source_line << ")";
    
    m_file << oss.str() << std::endl;
    m_file.flush();
}

// ============================================================================
// AsyncLogger Implementation
// ============================================================================

AsyncLogger& AsyncLogger::instance() {
    static AsyncLogger instance;
    return instance;
}

void AsyncLogger::initialize(std::string_view app_name, LogLevel min_level) {
    m_app_name = app_name;
    m_min_level = min_level;
    m_running = true;
    
    // Add console backend by default
    add_backend(std::make_unique<ConsoleBackend>());
    
    // Add file backend
    try {
        auto log_path = std::filesystem::current_path() / "logs" / 
                       std::format("{}_{}.log", app_name, 
                                   std::chrono::system_clock::now().time_since_epoch().count());
        add_backend(std::make_unique<FileBackend>(log_path));
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to create file logger: " << e.what() << std::endl;
    }
    
    m_worker_thread = std::jthread(&AsyncLogger::worker_thread_func, this);
}

void AsyncLogger::shutdown() {
    m_running = false;
    m_cv.notify_all();
    
    if (m_worker_thread.joinable()) {
        m_worker_thread.request_stop();
    }
}

void AsyncLogger::log(LogLevel level, std::string_view message,
                      const std::source_location& location) {
    if (level < m_min_level.load()) {
        return;
    }
    
    LogEntry entry{
        .timestamp = std::chrono::system_clock::now(),
        .level = level,
        .message = std::string(message),
        .source_file = std::string(location.file_name()),
        .source_line = location.line(),
        .function = std::string(location.function_name()),
        .thread_id = std::this_thread::get_id()
    };
    
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        
        // Drop oldest entries if queue is full (shouldn't happen in normal operation)
        if (m_queue.size() >= MAX_QUEUE_SIZE) {
            m_queue.pop();
        }
        
        m_queue.push(std::move(entry));
    }
    
    m_cv.notify_one();
}

void AsyncLogger::add_backend(std::unique_ptr<LoggerBackend> backend) {
    m_backends.push_back(std::move(backend));
}

void AsyncLogger::worker_thread_func() {
    while (m_running || !m_queue.empty()) {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        
        m_cv.wait(lock, [this] {
            return !m_queue.empty() || !m_running;
        });
        
        while (!m_queue.empty()) {
            auto entry = std::move(m_queue.front());
            m_queue.pop();
            lock.unlock();
            
            // Write to all backends
            for (auto& backend : m_backends) {
                try {
                    backend->write(entry);
                } catch (const std::exception& e) {
                    std::cerr << "Logger backend error: " << e.what() << std::endl;
                }
            }
            
            lock.lock();
        }
    }
}

std::string AsyncLogger::format_entry(const LogEntry& entry) const {
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count()
        << " [" << std::setw(5) << level_to_string(entry.level) << "] "
        << "[" << std::this_thread::get_id() << "] "
        << entry.message
        << " (" << std::filesystem::path(entry.source_file).filename().string()
        << ":" << entry.source_line << ")";
    
    return oss.str();
}

} // namespace kal::logger
