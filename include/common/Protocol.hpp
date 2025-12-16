#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace em {

// Frame wire format:
//   [MAGIC:2][TYPE:1][CMD:1][LEN:2][PAYLOAD:LEN bytes][CRC16:2]
// All multi-byte fields are big-endian.

constexpr uint8_t  MAGIC_HI       = 0x45; // 'E'
constexpr uint8_t  MAGIC_LO       = 0x4D; // 'M'
constexpr size_t   HEADER_LEN     = 6;    // magic + type + cmd + len
constexpr size_t   TRAILER_LEN    = 2;    // crc16
constexpr size_t   FRAME_OVERHEAD = HEADER_LEN + TRAILER_LEN;
constexpr size_t   MAX_PAYLOAD    = 256;

enum class MessageType : uint8_t {
    COMMAND  = 0x01,  // host  -> board
    RESPONSE = 0x02,  // board -> host (reply to command)
    STATUS   = 0x03,  // board -> host (unsolicited)
    ACK      = 0x04,
    NACK     = 0x05,
};

enum class CommandId : uint8_t {
    // Beam subsystem
    SET_BEAM_VOLTAGE  = 0x01,
    SET_BEAM_CURRENT  = 0x02,
    SET_FOCUS         = 0x03,
    ENABLE_BEAM       = 0x04,
    DISABLE_BEAM      = 0x05,
    GET_BEAM_PARAMS   = 0x06,
    // Stage subsystem
    SET_STAGE_X       = 0x10,
    SET_STAGE_Y       = 0x11,
    SET_STAGE_Z       = 0x12,
    SET_STAGE_TILT    = 0x13,
    GET_STAGE_PARAMS  = 0x14,
    // Imaging
    SET_MAGNIFICATION = 0x20,
    SET_SCAN_MODE     = 0x21,
    ACQUIRE_IMAGE     = 0x22,
    // System
    GET_STATUS        = 0x30,
    GET_VACUUM_STATUS = 0x31,
    EMERGENCY_STOP    = 0x32,
    RESET             = 0x33,
    UNKNOWN           = 0xFF,
};

enum class ResponseCode : uint8_t {
    OK                = 0x00,
    ERR_INVALID_CMD   = 0x01,
    ERR_OUT_OF_RANGE  = 0x02,
    ERR_INVALID_STATE = 0x03,
    ERR_BAD_CHECKSUM  = 0x04,
    ERR_VACUUM        = 0x05,
    ERR_HARDWARE      = 0x06,
};

enum class ScanMode : uint8_t {
    RASTER = 0x00,
    SPOT   = 0x01,
    LINE   = 0x02,
    FRAME  = 0x03,
};

enum class SystemState : uint8_t {
    IDLE      = 0x00,
    READY     = 0x01,
    IMAGING   = 0x02,
    ERROR     = 0x03,
    EMERGENCY = 0x04,
};

// Parsed frame passed between parser, handler, and transport.
struct Frame {
    MessageType          type;
    CommandId            cmd;
    std::vector<uint8_t> payload;

    std::vector<uint8_t> serialize() const;
};

// ── payload encoding helpers (big-endian) ────────────────────────────────────

inline void push_u8(std::vector<uint8_t>& b, uint8_t v) {
    b.push_back(v);
}
inline void push_u16(std::vector<uint8_t>& b, uint16_t v) {
    b.push_back(uint8_t(v >> 8));
    b.push_back(uint8_t(v));
}
inline void push_u32(std::vector<uint8_t>& b, uint32_t v) {
    b.push_back(uint8_t(v >> 24));
    b.push_back(uint8_t(v >> 16));
    b.push_back(uint8_t(v >> 8));
    b.push_back(uint8_t(v));
}
inline void push_f32(std::vector<uint8_t>& b, float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof bits);
    push_u32(b, bits);
}

inline uint8_t  pop_u8 (const uint8_t* p)             { return p[0]; }
inline uint16_t pop_u16(const uint8_t* p)             { return uint16_t(p[0] << 8) | p[1]; }
inline uint32_t pop_u32(const uint8_t* p)             {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16)
         | (uint32_t(p[2]) <<  8) |  uint32_t(p[3]);
}
inline float    pop_f32(const uint8_t* p) {
    uint32_t bits = pop_u32(p);
    float v;
    std::memcpy(&v, &bits, sizeof v);
    return v;
}

// CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF, no reflection)
uint16_t crc16(const uint8_t* data, size_t len);

} // namespace em
