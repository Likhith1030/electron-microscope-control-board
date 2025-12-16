#pragma once

#include <mutex>
#include <string>

namespace em {

enum class VacuumState { VENTING, PUMPING, READY, FAULT };

struct VacuumStatus {
    float       pressure_pa = 101325.0f;  // atmospheric at start
    VacuumState state       = VacuumState::PUMPING;
};

class VacuumSystem {
public:
    VacuumSystem();

    // Simulate one tick of vacuum pumping (call periodically).
    void tick();

    bool is_ready() const;
    VacuumStatus status() const;
    std::string status_string() const;

    // Operational threshold: beam safe below this pressure
    static constexpr float BEAM_SAFE_PA = 1e-3f;

private:
    mutable std::mutex mu_;
    VacuumStatus       status_;
};

} // namespace em
