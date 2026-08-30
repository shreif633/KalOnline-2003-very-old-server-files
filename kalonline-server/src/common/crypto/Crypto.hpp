#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace kal::crypto {

enum class PacketType : uint8_t {
    None = 0,
    Normal = 1,
    Critical = 2,
    Handshake = 3
};

class XorCipher {
public:
    explicit XorCipher(uint8_t key);
    
    void encrypt(std::span<uint8_t> data) const noexcept;
    void decrypt(std::span<uint8_t> data) const noexcept;
    
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) const;

private:
    uint8_t m_key;
};

class BlowfishCipher {
public:
    explicit BlowfishCipher(std::span<const uint8_t> key);
    
    void encrypt(std::span<uint8_t> data) const;
    void decrypt(std::span<uint8_t> data) const;
    
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) const;

private:
    static constexpr size_t BLOCK_SIZE = 8;
    
    std::vector<uint32_t> m_p;
    std::array<std::array<uint32_t, 256>, 4> m_s;
    
    void initialize(std::span<const uint8_t> key);
    uint32_t f(uint32_t x) const noexcept;
    void round(uint32_t& L, uint32_t& R) const noexcept;
    void encrypt_block(uint32_t& left, uint32_t& right) const;
    void decrypt_block(uint32_t& left, uint32_t& right) const;
    void encrypt_block(std::span<uint8_t, BLOCK_SIZE> block) const;
    void decrypt_block(std::span<uint8_t, BLOCK_SIZE> block) const;
};

class PacketCrypto {
public:
    PacketCrypto();
    
    void encrypt_packet(std::span<uint8_t> data, PacketType type);
    void decrypt_packet(std::span<uint8_t> data, PacketType type);

private:
    XorCipher m_xor;
    BlowfishCipher m_blowfish;
};

} // namespace kal::crypto
