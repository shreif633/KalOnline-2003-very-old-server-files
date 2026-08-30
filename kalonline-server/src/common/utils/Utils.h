#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <span>
#include <string_view>

namespace kal::utils {

// ============================================================================
// String Utilities
// ============================================================================

std::vector<std::string> split_string(const std::string& str, char delimiter);
std::string trim(const std::string& str);
std::string to_lower(std::string str);
std::string to_upper(std::string str);
bool starts_with(const std::string& str, const std::string& prefix);
bool ends_with(const std::string& str, const std::string& suffix);
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

// ============================================================================
// Buffer Writer/Reader
// ============================================================================

class BufferWriter {
public:
    void write_u8(uint8_t value);
    void write_u16(uint16_t value);
    void write_u32(uint32_t value);
    void write_string(const std::string& str);
    void write_string_fixed(const std::string& str, size_t length);
    void write_bytes(const uint8_t* data, size_t length);
    
    const std::vector<uint8_t>& buffer() const { return m_buffer; }
    std::vector<uint8_t>& buffer() { return m_buffer; }
    void clear();
    size_t size() const { return m_buffer.size(); }
    
private:
    std::vector<uint8_t> m_buffer;
};

class BufferReader {
public:
    explicit BufferReader(std::span<const uint8_t> data);
    
    uint8_t read_u8();
    uint16_t read_u16();
    uint32_t read_u32();
    std::string read_string(size_t length);
    std::string read_string_null_terminated();
    std::vector<uint8_t> read_bytes(size_t length);
    
    bool has_remaining() const { return m_pos < m_data.size(); }
    size_t position() const { return m_pos; }
    size_t remaining() const { return m_data.size() - m_pos; }
    
private:
    std::span<const uint8_t> m_data;
    size_t m_pos = 0;
};

// ============================================================================
// Hash Utilities
// ============================================================================

uint32_t hash_string(const std::string& str);
uint32_t hash_memory(const void* data, size_t length);

// ============================================================================
// Time Utilities
// ============================================================================

int64_t current_timestamp_ms();
int64_t current_timestamp_sec();

// ============================================================================
// File Utilities
// ============================================================================

std::vector<uint8_t> read_file_binary(const std::string& path);
std::string read_file_text(const std::string& path);
bool write_file_binary(const std::string& path, std::span<const uint8_t> data);
bool write_file_text(const std::string& path, std::string_view content);

} // namespace kal::utils
