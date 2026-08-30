#include "common/crypto/Crypto.hpp"
#include <algorithm>
#include <cstring>

namespace kal::crypto {

// ============================================================================
// XorCipher Implementation
// ============================================================================

void XorCipher::encrypt(std::span<uint8_t> data) const noexcept {
    for (auto& byte : data) {
        *byte ^= m_key;
    }
}

void XorCipher::decrypt(std::span<uint8_t> data) const noexcept {
    // XOR is symmetric - same operation for encrypt/decrypt
    encrypt(data);
}

// ============================================================================
// BlowfishCipher Implementation
// ============================================================================

// Original Blowfish S-boxes and P-array initialization constants
static constexpr uint32_t BLOWFISH_P_INIT[18] = {
    0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
    0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
    0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
    0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917,
    0x9216d5d9, 0x8979fb1b
};

static constexpr uint32_t BLOWFISH_S_INIT[4][256] = {
    // S[0] - truncated for brevity, full implementation would have all values
    {0xd1310ba6, 0x98dfb5ac, 0x2ffd72db, 0xd01adfb7, /* ... */},
    // S[1], S[2], S[3] similarly initialized
};

BlowfishCipher::BlowfishCipher(std::span<const uint8_t> key) {
    if (key.empty() || key.size() > MAX_KEY_SIZE) {
        throw std::invalid_argument("Blowfish key must be 1-56 bytes");
    }
    initialize(key);
}

void BlowfishCipher::initialize(std::span<const uint8_t> key) {
    // Initialize P-array and S-boxes with standard constants
    std::memcpy(m_p, BLOWFISH_P_INIT, sizeof(m_p));
    std::memcpy(m_s, BLOWFISH_S_INIT, sizeof(m_s));
    
    // XOR P-array with key bytes (repeating key as necessary)
    for (size_t i = 0; i < 18; ++i) {
        uint32_t key_word = 0;
        for (int j = 0; j < 4; ++j) {
            key_word = (key_word << 8) | key[(i * 4 + j) % key.size()];
        }
        m_p[i] ^= key_word;
    }
    
    // Encrypt all-zero string and replace P-array entries
    uint32_t L = 0, R = 0;
    for (int i = 0; i < 18; i += 2) {
        round(L, R);
        m_p[i] = L;
        m_p[i + 1] = R;
    }
    
    // Replace S-box entries
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 256; j += 2) {
            round(L, R);
            m_s[i][j] = L;
            m_s[i][j + 1] = R;
        }
    }
}

uint32_t BlowfishCipher::f(uint32_t x) const noexcept {
    uint32_t h = m_s[0][(x >> 24) & 0xFF];
    h += m_s[1][(x >> 16) & 0xFF];
    h ^= m_s[2][(x >> 8) & 0xFF];
    h += m_s[3][x & 0xFF];
    return h;
}

void BlowfishCipher::round(uint32_t& L, uint32_t& R) const noexcept {
    for (int i = 0; i < 16; ++i) {
        L ^= m_p[i + 1];
        R ^= f(L);
        std::swap(L, R);
    }
    std::swap(L, R); // Undo last swap
    R ^= m_p[17];
    L ^= m_p[16];
}

void BlowfishCipher::encrypt_block(std::span<uint8_t, BLOCK_SIZE> block) const {
    uint32_t L = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
    uint32_t R = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
    
    // Initial XOR with P-array
    L ^= m_p[0];
    
    // 16 rounds
    for (int i = 1; i <= 16; ++i) {
        R ^= f(L);
        std::swap(L, R);
        L ^= m_p[i];
    }
    
    // Final swap and XOR
    std::swap(L, R);
    R ^= m_p[17];
    
    // Convert back to bytes
    block[0] = (L >> 24) & 0xFF;
    block[1] = (L >> 16) & 0xFF;
    block[2] = (L >> 8) & 0xFF;
    block[3] = L & 0xFF;
    block[4] = (R >> 24) & 0xFF;
    block[5] = (R >> 16) & 0xFF;
    block[6] = (R >> 8) & 0xFF;
    block[7] = R & 0xFF;
}

void BlowfishCipher::decrypt_block(std::span<uint8_t, BLOCK_SIZE> block) const {
    uint32_t L = (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3];
    uint32_t R = (block[4] << 24) | (block[5] << 16) | (block[6] << 8) | block[7];
    
    // Reverse of encryption
    L ^= m_p[17];
    
    for (int i = 16; i >= 1; --i) {
        R ^= f(L);
        std::swap(L, R);
        L ^= m_p[i];
    }
    
    std::swap(L, R);
    R ^= m_p[0];
    
    block[0] = (L >> 24) & 0xFF;
    block[1] = (L >> 16) & 0xFF;
    block[2] = (L >> 8) & 0xFF;
    block[3] = L & 0xFF;
    block[4] = (R >> 24) & 0xFF;
    block[5] = (R >> 16) & 0xFF;
    block[6] = (R >> 8) & 0xFF;
    block[7] = R & 0xFF;
}

void BlowfishCipher::encrypt(std::span<uint8_t> data) const {
    // PKCS#5 padding and CBC mode would be implemented here
    // For packet encryption, we use ECB mode with padding
    size_t padded_size = ((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
    
    for (size_t i = 0; i < padded_size; i += BLOCK_SIZE) {
        std::span<uint8_t, BLOCK_SIZE> block(data.data() + i, BLOCK_SIZE);
        encrypt_block(block);
    }
}

void BlowfishCipher::decrypt(std::span<uint8_t> data) const {
    for (size_t i = 0; i < data.size(); i += BLOCK_SIZE) {
        std::span<uint8_t, BLOCK_SIZE> block(data.data() + i, BLOCK_SIZE);
        decrypt_block(block);
    }
}

// ============================================================================
// PacketCrypto Implementation
// ============================================================================

PacketCrypto::PacketCrypto() 
    : m_xor(XOR_KEY_2003)
    , m_blowfish(BLOWFISH_KEY_2003) {
}

void PacketCrypto::encrypt_packet(std::span<uint8_t> data, PacketType type) {
    switch (type) {
        case PacketType::Normal:
            m_xor.encrypt(data);
            break;
        case PacketType::Login:
        case PacketType::Critical:
            m_blowfish.encrypt(data);
            break;
        case PacketType::Handshake:
            // Handshake uses both: XOR first, then Blowfish
            m_xor.encrypt(data);
            m_blowfish.encrypt(data);
            break;
    }
}

void PacketCrypto::decrypt_packet(std::span<uint8_t> data, PacketType type) {
    switch (type) {
        case PacketType::Normal:
            m_xor.decrypt(data);
            break;
        case PacketType::Login:
        case PacketType::Critical:
            m_blowfish.decrypt(data);
            break;
        case PacketType::Handshake:
            // Reverse order: Blowfish first, then XOR
            m_blowfish.decrypt(data);
            m_xor.decrypt(data);
            break;
    }
}

} // namespace kal::crypto
