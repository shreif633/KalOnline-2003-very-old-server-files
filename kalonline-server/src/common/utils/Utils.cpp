#include "Utils.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdexcept>

namespace kal::utils {

// ============================================================================
// String Utilities
// ============================================================================

std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(std::move(token));
        }
    }
    
    return tokens;
}

std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();
    
    return (start < end) ? std::string(start, end) : std::string();
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool starts_with(const std::string& str, const std::string& prefix) {
    if (prefix.size() > str.size()) return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
    if (parts.empty()) return "";
    
    std::ostringstream oss;
    oss << parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        oss << delimiter << parts[i];
    }
    return oss.str();
}

// ============================================================================
// BufferWriter Implementation
// ============================================================================

void BufferWriter::write_u8(uint8_t value) {
    m_buffer.push_back(value);
}

void BufferWriter::write_u16(uint16_t value) {
    // Big-endian (network byte order)
    m_buffer.push_back((value >> 8) & 0xFF);
    m_buffer.push_back(value & 0xFF);
}

void BufferWriter::write_u32(uint32_t value) {
    // Big-endian
    m_buffer.push_back((value >> 24) & 0xFF);
    m_buffer.push_back((value >> 16) & 0xFF);
    m_buffer.push_back((value >> 8) & 0xFF);
    m_buffer.push_back(value & 0xFF);
}

void BufferWriter::write_string(const std::string& str) {
    write_u16(static_cast<uint16_t>(str.size()));
    m_buffer.insert(m_buffer.end(), str.begin(), str.end());
}

void BufferWriter::write_string_fixed(const std::string& str, size_t length) {
    size_t copy_len = std::min(str.size(), length);
    m_buffer.insert(m_buffer.end(), str.begin(), str.begin() + copy_len);
    
    // Pad with nulls
    for (size_t i = copy_len; i < length; ++i) {
        m_buffer.push_back(0);
    }
}

void BufferWriter::write_bytes(const uint8_t* data, size_t length) {
    m_buffer.insert(m_buffer.end(), data, data + length);
}

void BufferWriter::clear() {
    m_buffer.clear();
}

// ============================================================================
// BufferReader Implementation
// ============================================================================

BufferReader::BufferReader(std::span<const uint8_t> data)
    : m_data(data) {
}

uint8_t BufferReader::read_u8() {
    if (m_pos >= m_data.size()) {
        throw std::out_of_range("Buffer underflow reading uint8");
    }
    return m_data[m_pos++];
}

uint16_t BufferReader::read_u16() {
    if (m_pos + 2 > m_data.size()) {
        throw std::out_of_range("Buffer underflow reading uint16");
    }
    uint16_t value = (static_cast<uint16_t>(m_data[m_pos]) << 8) | m_data[m_pos + 1];
    m_pos += 2;
    return value;
}

uint32_t BufferReader::read_u32() {
    if (m_pos + 4 > m_data.size()) {
        throw std::out_of_range("Buffer underflow reading uint32");
    }
    uint32_t value = (static_cast<uint32_t>(m_data[m_pos]) << 24) |
                     (static_cast<uint32_t>(m_data[m_pos + 1]) << 16) |
                     (static_cast<uint32_t>(m_data[m_pos + 2]) << 8) |
                     m_data[m_pos + 3];
    m_pos += 4;
    return value;
}

std::string BufferReader::read_string(size_t length) {
    if (m_pos + length > m_data.size()) {
        throw std::out_of_range("Buffer underflow reading string");
    }
    std::string str(reinterpret_cast<const char*>(m_data.data() + m_pos), length);
    m_pos += length;
    return str;
}

std::string BufferReader::read_string_null_terminated() {
    auto start = m_data.begin() + m_pos;
    auto end = std::find(start, m_data.end(), '\0');
    
    std::string str(start, end);
    m_pos += (end - start) + 1;  // Include null terminator
    return str;
}

std::vector<uint8_t> BufferReader::read_bytes(size_t length) {
    if (m_pos + length > m_data.size()) {
        throw std::out_of_range("Buffer underflow reading bytes");
    }
    std::vector<uint8_t> bytes(m_data.begin() + m_pos, m_data.begin() + m_pos + length);
    m_pos += length;
    return bytes;
}

// ============================================================================
// Hash Utilities
// ============================================================================

uint32_t hash_string(const std::string& str) {
    // FNV-1a hash
    const uint32_t FNV_PRIME = 16777619;
    const uint32_t FNV_OFFSET = 2166136261;
    
    uint32_t hash = FNV_OFFSET;
    for (char c : str) {
        hash ^= static_cast<uint8_t>(c);
        hash *= FNV_PRIME;
    }
    return hash;
}

uint32_t hash_memory(const void* data, size_t length) {
    // FNV-1a hash
    const uint32_t FNV_PRIME = 16777619;
    const uint32_t FNV_OFFSET = 2166136261;
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t hash = FNV_OFFSET;
    
    for (size_t i = 0; i < length; ++i) {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

// ============================================================================
// Time Utilities
// ============================================================================

int64_t current_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int64_t current_timestamp_sec() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

// ============================================================================
// File Utilities
// ============================================================================

std::vector<uint8_t> read_file_binary(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + path);
    }
    
    return buffer;
}

std::string read_file_text(const std::string& path) {
    std::ifstream file(path, std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::string buffer(static_cast<size_t>(size), '\0');
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read file: " + path);
    }
    
    return buffer;
}

bool write_file_binary(const std::string& path, std::span<const uint8_t> data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), 
               static_cast<std::streamsize>(data.size()));
    return file.good();
}

bool write_file_text(const std::string& path, std::string_view content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return file.good();
}

} // namespace kal::utils
