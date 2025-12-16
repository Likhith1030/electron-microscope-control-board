#include "microscope/BeamController.hpp"
#include "common/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace em {

bool BeamController::set_voltage(float kV) {
    if (kV < VOLTAGE_MIN_KV || kV > VOLTAGE_MAX_KV) {
        LOG_WARN("BeamController", "Voltage out of range: " + std::to_string(kV) + " kV");
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    params_.voltage_kV = kV;
    LOG_INFO("BeamController", "Voltage set to " + std::to_string(kV) + " kV");
    return true;
}

bool BeamController::set_current(float nA) {
    if (nA < CURRENT_MIN_NA || nA > CURRENT_MAX_NA) {
        LOG_WARN("BeamController", "Current out of range: " + std::to_string(nA) + " nA");
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    params_.current_nA = nA;
    LOG_INFO("BeamController", "Current set to " + std::to_string(nA) + " nA");
    return true;
}

bool BeamController::set_focus(float mm) {
    if (mm < FOCUS_MIN_MM || mm > FOCUS_MAX_MM) {
        LOG_WARN("BeamController", "Focus out of range: " + std::to_string(mm) + " mm");
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    params_.focus_mm = mm;
    LOG_INFO("BeamController", "Focus set to " + std::to_string(mm) + " mm");
    return true;
}

bool BeamController::enable() {
    std::lock_guard<std::mutex> lk(mu_);
    if (params_.voltage_kV < VOLTAGE_MIN_KV) {
        LOG_WARN("BeamController", "Cannot enable beam: voltage not set");
        return false;
    }
    params_.beam_enabled = true;
    LOG_INFO("BeamController", "Beam ENABLED");
    return true;
}

bool BeamController::disable() {
    std::lock_guard<std::mutex> lk(mu_);
    params_.beam_enabled = false;
    LOG_INFO("BeamController", "Beam DISABLED");
    return true;
}

BeamParams BeamController::params() const {
    std::lock_guard<std::mutex> lk(mu_);
    return params_;
}

std::string BeamController::status_string() const {
    std::lock_guard<std::mutex> lk(mu_);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Beam[" << (params_.beam_enabled ? "ON" : "OFF") << "] "
       << "V=" << params_.voltage_kV << "kV "
       << "I=" << params_.current_nA << "nA "
       << "F=" << params_.focus_mm   << "mm";
    return ss.str();
}

} // namespace em
