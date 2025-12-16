#include "control_board/CommandHandler.hpp"
#include "common/Logger.hpp"
#include <sstream>

namespace em {

CommandHandler::CommandHandler(MicroscopeController& scope) : scope_(scope) {}

// ── routing ──────────────────────────────────────────────────────────────────

Frame CommandHandler::handle(const Frame& cmd) {
    switch (cmd.cmd) {
        case CommandId::SET_BEAM_VOLTAGE:   return handle_set_beam_voltage(cmd);
        case CommandId::SET_BEAM_CURRENT:   return handle_set_beam_current(cmd);
        case CommandId::SET_FOCUS:          return handle_set_focus(cmd);
        case CommandId::ENABLE_BEAM:        return handle_enable_beam(cmd);
        case CommandId::DISABLE_BEAM:       return handle_disable_beam(cmd);
        case CommandId::GET_BEAM_PARAMS:    return handle_get_beam_params(cmd);

        case CommandId::SET_STAGE_X:        return handle_set_stage_x(cmd);
        case CommandId::SET_STAGE_Y:        return handle_set_stage_y(cmd);
        case CommandId::SET_STAGE_Z:        return handle_set_stage_z(cmd);
        case CommandId::SET_STAGE_TILT:     return handle_set_stage_tilt(cmd);
        case CommandId::GET_STAGE_PARAMS:   return handle_get_stage_params(cmd);

        case CommandId::SET_MAGNIFICATION:  return handle_set_magnification(cmd);
        case CommandId::SET_SCAN_MODE:      return handle_set_scan_mode(cmd);
        case CommandId::ACQUIRE_IMAGE:      return handle_acquire_image(cmd);

        case CommandId::GET_STATUS:         return handle_get_status(cmd);
        case CommandId::GET_VACUUM_STATUS:  return handle_get_vacuum_status(cmd);
        case CommandId::EMERGENCY_STOP:     return handle_emergency_stop(cmd);
        case CommandId::RESET:              return handle_reset(cmd);

        default:
            LOG_WARN("CommandHandler", "Unknown command received");
            return err_response(cmd.cmd, ResponseCode::ERR_INVALID_CMD);
    }
}

// ── helpers ──────────────────────────────────────────────────────────────────

Frame CommandHandler::ok_response(CommandId cmd) {
    Frame f;
    f.type = MessageType::RESPONSE;
    f.cmd  = cmd;
    push_u8(f.payload, uint8_t(ResponseCode::OK));
    return f;
}

Frame CommandHandler::err_response(CommandId cmd, ResponseCode code) {
    Frame f;
    f.type = MessageType::RESPONSE;
    f.cmd  = cmd;
    push_u8(f.payload, uint8_t(code));
    return f;
}

// ── beam ─────────────────────────────────────────────────────────────────────

Frame CommandHandler::handle_set_beam_voltage(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    float kV = pop_f32(f.payload.data());
    return scope_.beam().set_voltage(kV)
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}

Frame CommandHandler::handle_set_beam_current(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    float nA = pop_f32(f.payload.data());
    return scope_.beam().set_current(nA)
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}

Frame CommandHandler::handle_set_focus(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    float mm = pop_f32(f.payload.data());
    return scope_.beam().set_focus(mm)
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}

Frame CommandHandler::handle_enable_beam(const Frame& f) {
    if (!scope_.vacuum().is_ready())
        return err_response(f.cmd, ResponseCode::ERR_VACUUM);
    return scope_.beam().enable()
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_INVALID_STATE);
}

Frame CommandHandler::handle_disable_beam(const Frame& f) {
    scope_.beam().disable();
    return ok_response(f.cmd);
}

Frame CommandHandler::handle_get_beam_params(const Frame& f) {
    auto p = scope_.beam().params();
    Frame resp;
    resp.type = MessageType::RESPONSE;
    resp.cmd  = f.cmd;
    push_u8 (resp.payload, uint8_t(ResponseCode::OK));
    push_f32(resp.payload, p.voltage_kV);
    push_f32(resp.payload, p.current_nA);
    push_f32(resp.payload, p.focus_mm);
    push_u8 (resp.payload, p.beam_enabled ? 1 : 0);
    return resp;
}

// ── stage ────────────────────────────────────────────────────────────────────

Frame CommandHandler::handle_set_stage_x(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    return scope_.stage().set_x(pop_f32(f.payload.data()))
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}
Frame CommandHandler::handle_set_stage_y(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    return scope_.stage().set_y(pop_f32(f.payload.data()))
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}
Frame CommandHandler::handle_set_stage_z(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    return scope_.stage().set_z(pop_f32(f.payload.data()))
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}
Frame CommandHandler::handle_set_stage_tilt(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    return scope_.stage().set_tilt(pop_f32(f.payload.data()))
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}
Frame CommandHandler::handle_get_stage_params(const Frame& f) {
    auto p = scope_.stage().params();
    Frame resp;
    resp.type = MessageType::RESPONSE;
    resp.cmd  = f.cmd;
    push_u8 (resp.payload, uint8_t(ResponseCode::OK));
    push_f32(resp.payload, p.x_um);
    push_f32(resp.payload, p.y_um);
    push_f32(resp.payload, p.z_um);
    push_f32(resp.payload, p.tilt_deg);
    return resp;
}

// ── imaging ──────────────────────────────────────────────────────────────────

Frame CommandHandler::handle_set_magnification(const Frame& f) {
    if (f.payload.size() < 4) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    return scope_.set_magnification(pop_f32(f.payload.data()))
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_OUT_OF_RANGE);
}
Frame CommandHandler::handle_set_scan_mode(const Frame& f) {
    if (f.payload.empty()) return err_response(f.cmd, ResponseCode::ERR_INVALID_CMD);
    scope_.set_scan_mode(static_cast<ScanMode>(f.payload[0]));
    return ok_response(f.cmd);
}
Frame CommandHandler::handle_acquire_image(const Frame& f) {
    return scope_.acquire_image()
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_INVALID_STATE);
}

// ── system ───────────────────────────────────────────────────────────────────

Frame CommandHandler::handle_get_status(const Frame& f) {
    Frame resp;
    resp.type = MessageType::RESPONSE;
    resp.cmd  = f.cmd;
    push_u8(resp.payload, uint8_t(ResponseCode::OK));
    push_u8(resp.payload, uint8_t(scope_.state()));
    push_u8(resp.payload, uint8_t(scope_.vacuum().status().state));
    auto bp = scope_.beam().params();
    push_u8(resp.payload, bp.beam_enabled ? 1 : 0);
    push_u32(resp.payload, scope_.imaging_params().frame_count);
    return resp;
}

Frame CommandHandler::handle_get_vacuum_status(const Frame& f) {
    auto vs = scope_.vacuum().status();
    Frame resp;
    resp.type = MessageType::RESPONSE;
    resp.cmd  = f.cmd;
    push_u8 (resp.payload, uint8_t(ResponseCode::OK));
    push_u8 (resp.payload, uint8_t(vs.state));
    push_f32(resp.payload, vs.pressure_pa);
    return resp;
}

Frame CommandHandler::handle_emergency_stop(const Frame& f) {
    scope_.emergency_stop();
    return ok_response(f.cmd);
}

Frame CommandHandler::handle_reset(const Frame& f) {
    return scope_.reset()
        ? ok_response(f.cmd)
        : err_response(f.cmd, ResponseCode::ERR_INVALID_STATE);
}

} // namespace em
