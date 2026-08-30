#pragma once

#include <string>
#include <cstdint>
#include <span>
#include <array>

namespace kal::crypto {

/**
 * @brief Original KalOnline XOR encryption key
 * Derived from reverse engineering the 2003 client/server protocol
 */
class XorCipher {
public:
    static constexpr uint8_t DEFAULT_KEY = 0x5A;
    
    explicit XorCipher(uint8_t key = DEFAULT_KEY) : m_key(key) {}
    
    void encrypt(std::span<uint8_t> data) const noexcept;
    void decrypt(std::span<uint8_t> data) const noexcept;
    
private:
    uint8_t m_key;
};

/**
 * @brief Blowfish encryption for sensitive packets
 * Used for login credentials and critical game data
 */
class BlowfishCipher {
public:
    static constexpr size_t BLOCK_SIZE = 8;
    static constexpr size_t MAX_KEY_SIZE = 56;
    
    explicit BlowfishCipher(std::span<const uint8_t> key);
    
    void encrypt_block(std::span<uint8_t, BLOCK_SIZE> block) const;
    void decrypt_block(std::span<uint8_t, BLOCK_SIZE> block) const;
    
    void encrypt(std::span<uint8_t> data) const;
    void decrypt(std::span<uint8_t> data) const;
    
private:
    void initialize(std::span<const uint8_t> key);
    
    uint32_t f(uint32_t x) const noexcept;
    void round(uint32_t& L, uint32_t& R) const noexcept;
    
    uint32_t m_s[4][256];
    uint32_t m_p[18];
};

/**
 * @brief Packet encryption handler
 * Combines XOR and Blowfish based on packet type
 */
class PacketCrypto {
public:
    enum class PacketType : uint8_t {
        Normal = 0,
        Login = 1,
        Critical = 2,
        Handshake = 3
    };
    
    PacketCrypto();
    
    void encrypt_packet(std::span<uint8_t> data, PacketType type);
    void decrypt_packet(std::span<uint8_t> data, PacketType type);
    
    // Original 2003 protocol keys
    static constexpr uint8_t XOR_KEY_2003 = 0x5A;
    static inline const std::array<uint8_t, 16> BLOWFISH_KEY_2003 = {
        0x4B, 0x61, 0x6C, 0x4F, 0x6E, 0x6C, 0x69, 0x6E,
        0x65, 0x32, 0x30, 0x30, 0x33, 0x47, 0x61, 0x6D
    }; // "KalOnline2003Gam"
    
private:
    XorCipher m_xor;
    BlowfishCipher m_blowfish;
};

} // namespace kal::crypto
