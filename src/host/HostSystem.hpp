#pragma once

#include "host/CommunicationChannel.hpp"
#include "common/Protocol.hpp"
#include <string>
#include <optional>
#include <chrono>

namespace em {

// Represents the host PC.  Builds command frames, sends them over the channel,
// and waits for the board's response.
class HostSystem {
public:
    explicit HostSystem(CommunicationChannel& ch,
                        std::chrono::milliseconds timeout = std::chrono::milliseconds{500});

    // Returns the response code from the board, or nullopt on timeout.
    std::optional<ResponseCode> set_beam_voltage(float kV);
    std::optional<ResponseCode> set_beam_current(float nA);
    std::optional<ResponseCode> set_focus(float mm);
    std::optional<ResponseCode> enable_beam();
    std::optional<ResponseCode> disable_beam();

    std::optional<ResponseCode> set_stage_x(float um);
    std::optional<ResponseCode> set_stage_y(float um);
    std::optional<ResponseCode> set_stage_z(float um);
    std::optional<ResponseCode> set_stage_tilt(float deg);

    std::optional<ResponseCode> set_magnification(float mag);
    std::optional<ResponseCode> set_scan_mode(ScanMode mode);
    std::optional<ResponseCode> acquire_image();

    std::optional<ResponseCode> get_status();
    std::optional<ResponseCode> get_vacuum_status();
    std::optional<ResponseCode> emergency_stop();
    std::optional<ResponseCode> reset();

private:
    std::optional<ResponseCode> send_f32_cmd(CommandId cmd, float value);
    std::optional<ResponseCode> send_u8_cmd (CommandId cmd, uint8_t value);
    std::optional<ResponseCode> send_empty_cmd(CommandId cmd);
    std::optional<ResponseCode> send_and_wait(const Frame& frame);

    CommunicationChannel&     ch_;
    std::chrono::milliseconds timeout_;
};

} // namespace em
