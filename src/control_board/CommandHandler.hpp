#pragma once

#include "common/Protocol.hpp"
#include "microscope/MicroscopeController.hpp"

namespace em {

// Dispatches a parsed command Frame to the appropriate subsystem
// and returns a response Frame ready to send back to the host.
class CommandHandler {
public:
    explicit CommandHandler(MicroscopeController& scope);

    Frame handle(const Frame& cmd);

private:
    Frame ok_response(CommandId cmd);
    Frame err_response(CommandId cmd, ResponseCode code);

    // Per-command handlers
    Frame handle_set_beam_voltage (const Frame& f);
    Frame handle_set_beam_current (const Frame& f);
    Frame handle_set_focus        (const Frame& f);
    Frame handle_enable_beam      (const Frame& f);
    Frame handle_disable_beam     (const Frame& f);
    Frame handle_get_beam_params  (const Frame& f);

    Frame handle_set_stage_x      (const Frame& f);
    Frame handle_set_stage_y      (const Frame& f);
    Frame handle_set_stage_z      (const Frame& f);
    Frame handle_set_stage_tilt   (const Frame& f);
    Frame handle_get_stage_params (const Frame& f);

    Frame handle_set_magnification(const Frame& f);
    Frame handle_set_scan_mode    (const Frame& f);
    Frame handle_acquire_image    (const Frame& f);

    Frame handle_get_status       (const Frame& f);
    Frame handle_get_vacuum_status(const Frame& f);
    Frame handle_emergency_stop   (const Frame& f);
    Frame handle_reset            (const Frame& f);

    MicroscopeController& scope_;
};

} // namespace em
