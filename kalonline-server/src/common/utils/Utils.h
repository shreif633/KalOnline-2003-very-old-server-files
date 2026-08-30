#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace kal::utils {

std::vector<std::string> split_string(const std::string& str, char delimiter);
std::string trim(const std::string& str);
std::string to_lower(const std::string& str);
std::string to_upper(const std::string& str);
bool starts_with(const std::string& str, const std::string& prefix);
bool ends_with(const std::string& str, const std::string& suffix);
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

int64_t get_timestamp_ms();
std::string format_bytes(uint64_t bytes);

} // namespace kal::utils
