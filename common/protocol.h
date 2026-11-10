#pragma once

#include <cstdint>
#include <stdexcept>

namespace Protocol
{
    constexpr uint32_t MAX_MESSAGE_SIZE = 64 * 1024;
    constexpr uint32_t HEADER_SIZE = 4;

    // Encode length in big-endian byte order
    inline void encodeLength(
        uint32_t length,
        unsigned char* buffer)
    {
        buffer[0] = (length >> 24) & 0xFF;
        buffer[1] = (length >> 16) & 0xFF;
        buffer[2] = (length >> 8) & 0xFF;
        buffer[3] = length & 0xFF;
    }

    inline uint32_t decodeLength(
        const unsigned char* buffer)
    {
        return
            (uint32_t(buffer[0]) << 24) |
            (uint32_t(buffer[1]) << 16) |
            (uint32_t(buffer[2]) << 8) |
            uint32_t(buffer[3]);
    }

    inline bool isValidLength(uint32_t length)
    {
        return length <= MAX_MESSAGE_SIZE;
    }
}