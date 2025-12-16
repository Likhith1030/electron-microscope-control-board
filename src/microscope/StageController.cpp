#include "microscope/StageController.hpp"
#include "common/Logger.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace em {

bool StageController::set_x(float um) {
    if (std::abs(um) > XY_LIMIT_UM) {
        LOG_WARN("StageController", "X out of range: " + std::to_string(um));
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    params_.x_um = um;
    LOG_INFO("StageController", "X = " + std::to_string(um) + " µm");
    return true;
}

bool StageController::set_y(float um) {
    if (std::abs(um) > XY_LIMIT_UM) {
        LOG_WARN("StageController", "Y out of range: " + std::to_string(um));
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    params_.y_um = um;
    LOG_INFO("StageController", "Y = " + std::to_string(um) + " µm");
    return true;
}

bool StageController::set_z(float um) {
    if (um < Z_MIN_UM || um > Z_MAX_UM) {
        LOG_WARN("StageController", "Z out of range: " + std::to_string(um));
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    params_.z_um = um;
    LOG_INFO("StageController", "Z = " + std::to_string(um) + " µm");
    return true;
}

bool StageController::set_tilt(float deg) {
    if (std::abs(deg) > TILT_LIMIT_DEG) {
        LOG_WARN("StageController", "Tilt out of range: " + std::to_string(deg));
        return false;
    }
    std::lock_guard<std::mutex> lk(mu_);
    params_.tilt_deg = deg;
    LOG_INFO("StageController", "Tilt = " + std::to_string(deg) + "°");
    return true;
}

StageParams StageController::params() const {
    std::lock_guard<std::mutex> lk(mu_);
    return params_;
}

std::string StageController::status_string() const {
    std::lock_guard<std::mutex> lk(mu_);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "Stage X=" << params_.x_um    << "µm "
       << "Y="       << params_.y_um    << "µm "
       << "Z="       << params_.z_um    << "µm "
       << "Tilt="    << params_.tilt_deg << "°";
    return ss.str();
}

} // namespace em
