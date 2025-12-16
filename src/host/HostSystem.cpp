#include "host/HostSystem.hpp"
#include "common/Logger.hpp"
#include "control_board/MessageParser.hpp"

namespace em {

HostSystem::HostSystem(CommunicationChannel& ch, std::chrono::milliseconds timeout)
    : ch_(ch), timeout_(timeout) {}

// ── helpers ──────────────────────────────────────────────────────────────────

std::optional<ResponseCode> HostSystem::send_and_wait(const Frame& frame) {
    ch_.host_to_board().write(frame.serialize());

    auto pkt = ch_.board_to_host().read(timeout_);
    if (!pkt) {
        LOG_WARN("HostSystem", "Timeout waiting for response");
        return std::nullopt;
    }

    // Parse the single-packet response
    std::optional<ResponseCode> result;
    MessageParser parser([&](const Frame& resp) {
        if (!resp.payload.empty())
            result = static_cast<ResponseCode>(resp.payload[0]);
    });
    parser.feed(*pkt);
    return result;
}

std::optional<ResponseCode> HostSystem::send_f32_cmd(CommandId cmd, float value) {
    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = cmd;
    push_f32(f.payload, value);
    return send_and_wait(f);
}

std::optional<ResponseCode> HostSystem::send_u8_cmd(CommandId cmd, uint8_t value) {
    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = cmd;
    push_u8(f.payload, value);
    return send_and_wait(f);
}

std::optional<ResponseCode> HostSystem::send_empty_cmd(CommandId cmd) {
    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = cmd;
    return send_and_wait(f);
}

// ── beam ─────────────────────────────────────────────────────────────────────

std::optional<ResponseCode> HostSystem::set_beam_voltage(float kV) {
    return send_f32_cmd(CommandId::SET_BEAM_VOLTAGE, kV);
}
std::optional<ResponseCode> HostSystem::set_beam_current(float nA) {
    return send_f32_cmd(CommandId::SET_BEAM_CURRENT, nA);
}
std::optional<ResponseCode> HostSystem::set_focus(float mm) {
    return send_f32_cmd(CommandId::SET_FOCUS, mm);
}
std::optional<ResponseCode> HostSystem::enable_beam() {
    return send_empty_cmd(CommandId::ENABLE_BEAM);
}
std::optional<ResponseCode> HostSystem::disable_beam() {
    return send_empty_cmd(CommandId::DISABLE_BEAM);
}

// ── stage ────────────────────────────────────────────────────────────────────

std::optional<ResponseCode> HostSystem::set_stage_x(float um) {
    return send_f32_cmd(CommandId::SET_STAGE_X, um);
}
std::optional<ResponseCode> HostSystem::set_stage_y(float um) {
    return send_f32_cmd(CommandId::SET_STAGE_Y, um);
}
std::optional<ResponseCode> HostSystem::set_stage_z(float um) {
    return send_f32_cmd(CommandId::SET_STAGE_Z, um);
}
std::optional<ResponseCode> HostSystem::set_stage_tilt(float deg) {
    return send_f32_cmd(CommandId::SET_STAGE_TILT, deg);
}

// ── imaging ──────────────────────────────────────────────────────────────────

std::optional<ResponseCode> HostSystem::set_magnification(float mag) {
    return send_f32_cmd(CommandId::SET_MAGNIFICATION, mag);
}
std::optional<ResponseCode> HostSystem::set_scan_mode(ScanMode mode) {
    return send_u8_cmd(CommandId::SET_SCAN_MODE, uint8_t(mode));
}
std::optional<ResponseCode> HostSystem::acquire_image() {
    return send_empty_cmd(CommandId::ACQUIRE_IMAGE);
}

// ── system ───────────────────────────────────────────────────────────────────

std::optional<ResponseCode> HostSystem::get_status() {
    return send_empty_cmd(CommandId::GET_STATUS);
}
std::optional<ResponseCode> HostSystem::get_vacuum_status() {
    return send_empty_cmd(CommandId::GET_VACUUM_STATUS);
}
std::optional<ResponseCode> HostSystem::emergency_stop() {
    return send_empty_cmd(CommandId::EMERGENCY_STOP);
}
std::optional<ResponseCode> HostSystem::reset() {
    return send_empty_cmd(CommandId::RESET);
}

} // namespace em
