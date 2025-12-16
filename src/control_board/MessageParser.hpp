#pragma once

#include "common/Protocol.hpp"
#include <functional>
#include <vector>
#include <optional>

namespace em {

// Byte-at-a-time state-machine parser.
// Feed raw bytes with feed(); complete frames are delivered via the callback.
class MessageParser {
public:
    using FrameCallback = std::function<void(const Frame&)>;

    explicit MessageParser(FrameCallback cb);

    // Ingest one byte; fires callback when a valid frame is assembled.
    void feed(uint8_t byte);

    // Feed a whole buffer.
    void feed(const uint8_t* data, size_t len);
    void feed(const std::vector<uint8_t>& data);

    // Number of frames parsed successfully.
    uint64_t frames_parsed()   const { return frames_parsed_;   }
    // Number of frames rejected (bad checksum / oversize payload).
    uint64_t frames_rejected() const { return frames_rejected_; }

    void reset();

private:
    enum class State {
        MAGIC_HI,
        MAGIC_LO,
        TYPE,
        CMD,
        LEN_HI,
        LEN_LO,
        PAYLOAD,
        CRC_HI,
        CRC_LO,
    };

    void dispatch();

    FrameCallback cb_;
    State         state_{State::MAGIC_HI};
    Frame         current_;
    uint16_t      payload_remaining_{0};
    uint8_t       crc_hi_{0};
    uint64_t      frames_parsed_{0};
    uint64_t      frames_rejected_{0};
};

} // namespace em
