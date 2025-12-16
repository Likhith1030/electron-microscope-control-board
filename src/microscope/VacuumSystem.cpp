#include "microscope/VacuumSystem.hpp"
#include "common/Logger.hpp"
#include <sstream>
#include <iomanip>

namespace em {

VacuumSystem::VacuumSystem() {
    status_.pressure_pa = 101325.0f;
    status_.state       = VacuumState::PUMPING;
    LOG_INFO("VacuumSystem", "Initialised – pumping down from atmosphere");
}

void VacuumSystem::tick() {
    std::lock_guard<std::mutex> lk(mu_);
    if (status_.state == VacuumState::PUMPING) {
        // Exponential decay model: pressure halves each tick
        status_.pressure_pa *= 0.55f;
        if (status_.pressure_pa <= BEAM_SAFE_PA) {
            status_.pressure_pa = BEAM_SAFE_PA * 0.1f; // settle below threshold
            status_.state       = VacuumState::READY;
            LOG_INFO("VacuumSystem", "Vacuum READY – beam safe");
        }
    }
}

bool VacuumSystem::is_ready() const {
    std::lock_guard<std::mutex> lk(mu_);
    return status_.state == VacuumState::READY;
}

VacuumStatus VacuumSystem::status() const {
    std::lock_guard<std::mutex> lk(mu_);
    return status_;
}

std::string VacuumSystem::status_string() const {
    std::lock_guard<std::mutex> lk(mu_);
    const char* st =
        status_.state == VacuumState::READY   ? "READY"   :
        status_.state == VacuumState::PUMPING ? "PUMPING" :
        status_.state == VacuumState::VENTING ? "VENTING" : "FAULT";
    std::ostringstream ss;
    ss << std::scientific << std::setprecision(2);
    ss << "Vacuum[" << st << "] P=" << status_.pressure_pa << " Pa";
    return ss.str();
}

} // namespace em
