#include "control_board/MessageParser.hpp"
#include "common/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace em {

// CRC-16/CCITT-FALSE: poly=0x1021, init=0xFFFF, no input/output reflection
uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= uint16_t(data[i]) << 8;
        for (int j = 0; j < 8; ++j)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

// Frame::serialize -----------------------------------------------------------

std::vector<uint8_t> Frame::serialize() const {
    std::vector<uint8_t> buf;
    buf.reserve(FRAME_OVERHEAD + payload.size());

    buf.push_back(MAGIC_HI);
    buf.push_back(MAGIC_LO);
    buf.push_back(uint8_t(type));
    buf.push_back(uint8_t(cmd));
    push_u16(buf, uint16_t(payload.size()));
    buf.insert(buf.end(), payload.begin(), payload.end());

    // CRC covers everything except the CRC field itself
    uint16_t crc = crc16(buf.data(), buf.size());
    push_u16(buf, crc);
    return buf;
}

// MessageParser --------------------------------------------------------------

MessageParser::MessageParser(FrameCallback cb) : cb_(std::move(cb)) {}

void MessageParser::reset() {
    state_ = State::MAGIC_HI;
    current_.payload.clear();
    payload_remaining_ = 0;
}

void MessageParser::feed(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) feed(data[i]);
}

void MessageParser::feed(const std::vector<uint8_t>& data) {
    feed(data.data(), data.size());
}

void MessageParser::feed(uint8_t byte) {
    switch (state_) {
        case State::MAGIC_HI:
            if (byte == MAGIC_HI) state_ = State::MAGIC_LO;
            break;

        case State::MAGIC_LO:
            if (byte == MAGIC_LO) { state_ = State::TYPE; }
            else                  { state_ = State::MAGIC_HI; }  // re-sync
            break;

        case State::TYPE:
            current_.type = static_cast<MessageType>(byte);
            state_ = State::CMD;
            break;

        case State::CMD:
            current_.cmd = static_cast<CommandId>(byte);
            state_ = State::LEN_HI;
            break;

        case State::LEN_HI:
            payload_remaining_ = uint16_t(byte) << 8;
            state_ = State::LEN_LO;
            break;

        case State::LEN_LO:
            payload_remaining_ |= byte;
            current_.payload.clear();
            if (payload_remaining_ > MAX_PAYLOAD) {
                LOG_WARN("MessageParser", "Payload too large, discarding frame");
                ++frames_rejected_;
                state_ = State::MAGIC_HI;
            } else if (payload_remaining_ == 0) {
                state_ = State::CRC_HI;
            } else {
                current_.payload.reserve(payload_remaining_);
                state_ = State::PAYLOAD;
            }
            break;

        case State::PAYLOAD:
            current_.payload.push_back(byte);
            if (--payload_remaining_ == 0) state_ = State::CRC_HI;
            break;

        case State::CRC_HI:
            crc_hi_ = byte;
            state_  = State::CRC_LO;
            break;

        case State::CRC_LO: {
            uint16_t received = uint16_t(crc_hi_ << 8) | byte;
            dispatch();          // fills buf for CRC computation
            auto wire = current_.serialize();
            // CRC is over everything except the last two bytes (the CRC itself)
            uint16_t expected = crc16(wire.data(), wire.size() - 2);
            if (received == expected) {
                ++frames_parsed_;
                cb_(current_);
            } else {
                ++frames_rejected_;
                std::ostringstream ss;
                ss << "CRC mismatch: expected 0x" << std::hex << std::setw(4)
                   << std::setfill('0') << expected << " got 0x" << received;
                LOG_WARN("MessageParser", ss.str());
            }
            state_ = State::MAGIC_HI;
            break;
        }
    }
}

void MessageParser::dispatch() {
    // payload already in current_; nothing else to do here —
    // serialization for CRC verification happens in the caller.
}

} // namespace em
