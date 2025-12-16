#include "microscope/MicroscopeController.hpp"
#include "common/Logger.hpp"
#include <sstream>

namespace em {

MicroscopeController::MicroscopeController() {
    LOG_INFO("MicroscopeController", "Initialising system");
    // Pump down the vacuum on construction; run until pressure drops below threshold.
    while (!vacuum_.is_ready()) vacuum_.tick();
    if (vacuum_.is_ready()) set_state(SystemState::READY);
    LOG_INFO("MicroscopeController", "System ready");
}

bool MicroscopeController::set_magnification(float mag) {
    if (mag < MAG_MIN || mag > MAG_MAX) {
        LOG_WARN("MicroscopeController", "Magnification out of range: " + std::to_string(mag));
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    imaging_.magnification = mag;
    LOG_INFO("MicroscopeController", "Magnification = " + std::to_string(mag) + "x");
    return true;
}

bool MicroscopeController::set_scan_mode(ScanMode mode) {
    std::lock_guard<std::mutex> lk(mu_);
    imaging_.scan_mode = mode;
    const char* names[] = {"RASTER", "SPOT", "LINE", "FRAME"};
    LOG_INFO("MicroscopeController", std::string("Scan mode = ") + names[uint8_t(mode)]);
    return true;
}

bool MicroscopeController::acquire_image() {
    if (state() != SystemState::READY) {
        LOG_WARN("MicroscopeController", "Cannot acquire: system not READY");
        return false;
    }
    if (!beam_.params().beam_enabled) {
        LOG_WARN("MicroscopeController", "Cannot acquire: beam disabled");
        return false;
    }
    set_state(SystemState::IMAGING);
    {
        std::lock_guard<std::mutex> lk(mu_);
        ++imaging_.frame_count;
        LOG_INFO("MicroscopeController",
                 "Acquiring frame #" + std::to_string(imaging_.frame_count));
    }
    set_state(SystemState::READY);
    return true;
}

bool MicroscopeController::emergency_stop() {
    LOG_ERROR("MicroscopeController", "EMERGENCY STOP triggered");
    beam_.disable();
    set_state(SystemState::EMERGENCY);
    return true;
}

bool MicroscopeController::reset() {
    if (state() == SystemState::EMERGENCY) {
        LOG_INFO("MicroscopeController", "Resetting from EMERGENCY state");
        set_state(SystemState::READY);
        return true;
    }
    LOG_WARN("MicroscopeController", "Reset called but not in EMERGENCY state");
    return false;
}

SystemState MicroscopeController::state() const {
    std::lock_guard<std::mutex> lk(mu_);
    return state_;
}

ImagingParams MicroscopeController::imaging_params() const {
    std::lock_guard<std::mutex> lk(mu_);
    return imaging_;
}

std::string MicroscopeController::full_status() const {
    const char* state_names[] = {"IDLE","READY","IMAGING","ERROR","EMERGENCY"};
    std::ostringstream ss;
    ss << "=== Microscope Status ===\n"
       << "  System : " << state_names[uint8_t(state())] << "\n"
       << "  " << beam_.status_string()  << "\n"
       << "  " << stage_.status_string() << "\n"
       << "  " << vacuum_.status_string() << "\n"
       << "  Mag="   << imaging_params().magnification << "x"
       << "  Frames=" << imaging_params().frame_count;
    return ss.str();
}

void MicroscopeController::set_state(SystemState s) {
    std::lock_guard<std::mutex> lk(mu_);
    state_ = s;
}

} // namespace em
