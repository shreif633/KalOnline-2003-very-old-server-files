#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace kal::utils {

/**
 * @brief Utility functions for common operations
 */

// Byte order conversion (network byte order)
inline uint16_t hton16(uint16_t hostshort) {
    return ((hostshort & 0x00FF) << 8) | ((hostshort & 0xFF00) >> 8);
}

inline uint32_t hton32(uint32_t hostlong) {
    return ((hostlong & 0x000000FF) << 24) |
           ((hostlong & 0x0000FF00) << 8) |
           ((hostlong & 0x00FF0000) >> 8) |
           ((hostlong & 0xFF000000) >> 24);
}

inline uint16_t ntoh16(uint16_t netshort) {
    return hton16(netshort);  // Same operation
}

inline uint32_t ntoh32(uint32_t netlong) {
    return hton32(netlong);  // Same operation
}

// String utilities
[[nodiscard]] std::vector<std::string> split_string(const std::string& str, char delimiter);
[[nodiscard]] std::string trim(const std::string& str);
[[nodiscard]] std::string to_lower(std::string str);
[[nodiscard]] std::string to_upper(std::string str);

// Buffer utilities
class BufferWriter {
public:
    void write_u8(uint8_t value);
    void write_u16(uint16_t value);
    void write_u32(uint32_t value);
    void write_string(const std::string& str);
    void write_string_fixed(const std::string& str, size_t length);
    void write_bytes(const uint8_t* data, size_t length);
    
    [[nodiscard]] const std::vector<uint8_t>& data() const noexcept { return m_buffer; }
    [[nodiscard]] std::vector<uint8_t>&& take_data() noexcept { return std::move(m_buffer); }
    void clear();
    
private:
    std::vector<uint8_t> m_buffer;
};

class BufferReader {
public:
    explicit BufferReader(std::span<const uint8_t> data);
    
    [[nodiscard]] uint8_t read_u8();
    [[nodiscard]] uint16_t read_u16();
    [[nodiscard]] uint32_t read_u32();
    [[nodiscard]] std::string read_string(size_t length);
    [[nodiscard]] std::string read_string_null_terminated();
    [[nodiscard]] std::vector<uint8_t> read_bytes(size_t length);
    
    [[nodiscard]] bool has_more() const noexcept { return m_pos < m_data.size(); }
    [[nodiscard]] size_t remaining() const noexcept { return m_data.size() - m_pos; }
    [[nodiscard]] size_t position() const noexcept { return m_pos; }
    void seek(size_t pos) { m_pos = pos; }
    void skip(size_t count) { m_pos += count; }
    
private:
    std::span<const uint8_t> m_data;
    size_t m_pos{0};
};

// Hash utilities
[[nodiscard]] uint32_t hash_string(const std::string& str);
[[nodiscard]] uint32_t hash_memory(const void* data, size_t length);

// Time utilities
[[nodiscard]] int64_t current_timestamp_ms();
[[nodiscard]] int64_t current_timestamp_sec();

// File utilities
[[nodiscard]] std::vector<uint8_t> read_file_binary(const std::string& path);
[[nodiscard]] std::string read_file_text(const std::string& path);
bool write_file_binary(const std::string& path, std::span<const uint8_t> data);
bool write_file_text(const std::string& path, std::string_view content);

// Math utilities
template<typename T>
constexpr T clamp(T value, T min_val, T max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

template<typename T>
constexpr T lerp(T a, T b, float t) {
    return a + static_cast<T>((b - a) * t);
}

inline float distance_2d(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

inline float distance_squared_2d(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

} // namespace kal::utils
