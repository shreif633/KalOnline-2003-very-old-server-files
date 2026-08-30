#pragma once
#include "Protocol.h"
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>

namespace kal {

/**
 * @brief Low-level binary packet builder for exact protocol compliance
 * 
 * Handles Little Endian byte order, EUC-KR string encoding, and proper padding
 * to match the original 2003 KalOnline client/server communication exactly.
 */
class PacketBuilder {
public:
    PacketBuilder() : buffer_(), writePos_(0) {
        // Reserve space for header initially
        buffer_.resize(sizeof(PacketHeader));
        writePos_ = sizeof(PacketHeader);
    }

    explicit PacketBuilder(uint16_t opcode) : buffer_(), writePos_(0) {
        buffer_.resize(sizeof(PacketHeader));
        writePos_ = sizeof(PacketHeader);
        WriteOpcode(opcode);
    }

    // --- Primitive Writers ---
    
    void WriteUInt8(uint8_t value) {
        buffer_.resize(writePos_ + sizeof(value));
        buffer_[writePos_] = value;
        writePos_ += sizeof(value);
    }

    void WriteUInt16(uint16_t value) {
        buffer_.resize(writePos_ + sizeof(value));
        // Little Endian
        buffer_[writePos_] = static_cast<uint8_t>(value & 0xFF);
        buffer_[writePos_ + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        writePos_ += sizeof(value);
    }

    void WriteUInt32(uint32_t value) {
        buffer_.resize(writePos_ + sizeof(value));
        // Little Endian
        buffer_[writePos_] = static_cast<uint8_t>(value & 0xFF);
        buffer_[writePos_ + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer_[writePos_ + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer_[writePos_ + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
        writePos_ += sizeof(value);
    }

    void WriteInt32(int32_t value) {
        WriteUInt32(static_cast<uint32_t>(value));
    }

    void WriteFloat(float value) {
        uint32_t intValue;
        std::memcpy(&intValue, &value, sizeof(float));
        WriteUInt32(intValue);
    }

    /**
     * @brief Writes a EUC-KR encoded string (null-terminated)
     * 
     * The original KalOnline uses EUC-KR for Korean character support.
     * This function assumes the input string is already properly encoded.
     */
    void WriteStringKR(const std::string& str, size_t maxLength = 0) {
        if (maxLength > 0) {
            // Fixed-length string with null padding
            size_t copyLen = std::min(str.length(), maxLength - 1);
            for (size_t i = 0; i < maxLength; ++i) {
                uint8_t ch = (i < copyLen) ? static_cast<uint8_t>(str[i]) : 0;
                WriteUInt8(ch);
            }
        } else {
            // Null-terminated variable-length string
            for (char c : str) {
                WriteUInt8(static_cast<uint8_t>(c));
            }
            WriteUInt8(0); // Null terminator
        }
    }

    void WriteBytes(const uint8_t* data, size_t len) {
        buffer_.resize(writePos_ + len);
        std::memcpy(buffer_.data() + writePos_, data, len);
        writePos_ += len;
    }

    // --- Specialized Writers ---

    void WriteOpcode(uint16_t opcode) {
        // Opcode goes after size in header
        if (buffer_.size() >= sizeof(PacketHeader)) {
            buffer_[2] = static_cast<uint8_t>(opcode & 0xFF);
            buffer_[3] = static_cast<uint8_t>((opcode >> 8) & 0xFF);
        }
    }

    void WriteCoord(float coord) {
        // Coordinates are sent as floats in the original protocol
        WriteFloat(coord);
    }

    void WritePosition(float x, float y, float z) {
        WriteCoord(x);
        WriteCoord(y);
        WriteCoord(z);
    }

    /**
     * @brief Finalizes the packet by writing the total size to the header
     * @return Reference to the internal buffer containing the complete packet
     */
    const std::vector<uint8_t>& Finish() {
        // Write total packet size (including header)
        uint16_t totalSize = static_cast<uint16_t>(buffer_.size());
        buffer_[0] = static_cast<uint8_t>(totalSize & 0xFF);
        buffer_[1] = static_cast<uint8_t>((totalSize >> 8) & 0xFF);
        
        return buffer_;
    }

    const std::vector<uint8_t>& GetBuffer() const { return buffer_; }
    size_t GetSize() const { return writePos_; }

private:
    std::vector<uint8_t> buffer_;
    size_t writePos_;
};

/**
 * @brief Low-level binary packet reader for parsing incoming client packets
 * 
 * Handles Little Endian byte order and EUC-KR string decoding to match
 * the original 2003 KalOnline client protocol exactly.
 */
class PacketReader {
public:
    explicit PacketReader(const uint8_t* data, size_t size)
        : data_(data), size_(size), readPos_(0) {}

    explicit PacketReader(const std::vector<uint8_t>& buffer)
        : data_(buffer.data()), size_(buffer.size()), readPos_(0) {}

    bool CanRead(size_t bytes) const {
        return (readPos_ + bytes) <= size_;
    }

    // --- Primitive Readers ---

    uint8_t ReadUInt8() {
        if (!CanRead(sizeof(uint8_t))) return 0;
        return data_[readPos_++];
    }

    uint16_t ReadUInt16() {
        if (!CanRead(sizeof(uint16_t))) return 0;
        uint16_t value = static_cast<uint16_t>(data_[readPos_]);
        value |= static_cast<uint16_t>(data_[readPos_ + 1]) << 8;
        readPos_ += sizeof(uint16_t);
        return value;
    }

    uint32_t ReadUInt32() {
        if (!CanRead(sizeof(uint32_t))) return 0;
        uint32_t value = static_cast<uint32_t>(data_[readPos_]);
        value |= static_cast<uint32_t>(data_[readPos_ + 1]) << 8;
        value |= static_cast<uint32_t>(data_[readPos_ + 2]) << 16;
        value |= static_cast<uint32_t>(data_[readPos_ + 3]) << 24;
        readPos_ += sizeof(uint32_t);
        return value;
    }

    int32_t ReadInt32() {
        return static_cast<int32_t>(ReadUInt32());
    }

    float ReadFloat() {
        uint32_t intValue = ReadUInt32();
        float floatValue;
        std::memcpy(&floatValue, &intValue, sizeof(float));
        return floatValue;
    }

    /**
     * @brief Reads a null-terminated EUC-KR string
     * 
     * Returns the string as-is (assumes caller handles EUC-KR encoding if needed)
     */
    std::string ReadStringKR() {
        std::string result;
        while (CanRead(1)) {
            uint8_t ch = ReadUInt8();
            if (ch == 0) break; // Null terminator
            result.push_back(static_cast<char>(ch));
        }
        return result;
    }

    /**
     * @brief Reads a fixed-length string (may include null padding)
     */
    std::string ReadStringFixed(size_t length) {
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length && CanRead(1); ++i) {
            uint8_t ch = ReadUInt8();
            if (ch != 0) {
                result.push_back(static_cast<char>(ch));
            }
        }
        return result;
    }

    void ReadBytes(uint8_t* dest, size_t len) {
        if (!CanRead(len)) return;
        std::memcpy(dest, data_ + readPos_, len);
        readPos_ += len;
    }

    // --- Specialized Readers ---

    PacketHeader ReadHeader() {
        PacketHeader header;
        header.size = ReadUInt16();
        header.opcode = ReadUInt16();
        return header;
    }

    float ReadCoord() {
        return ReadFloat();
    }

    void ReadPosition(float& x, float& y, float& z) {
        x = ReadCoord();
        y = ReadCoord();
        z = ReadCoord();
    }

    size_t GetReadPos() const { return readPos_; }
    size_t GetSize() const { return size_; }
    bool IsEof() const { return readPos_ >= size_; }

private:
    const uint8_t* data_;
    size_t size_;
    size_t readPos_;
};

} // namespace kal
