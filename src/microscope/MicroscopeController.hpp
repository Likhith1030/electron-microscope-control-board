#pragma once

#include "microscope/BeamController.hpp"
#include "microscope/StageController.hpp"
#include "microscope/VacuumSystem.hpp"
#include "common/Protocol.hpp"
#include <string>
#include <mutex>
#include <atomic>

namespace em {

struct ImagingParams {
    float    magnification = 1000.0f;
    ScanMode scan_mode     = ScanMode::RASTER;
    uint32_t frame_count   = 0;
};

class MicroscopeController {
public:
    MicroscopeController();

    // Subsystem accessors (used by CommandHandler)
    BeamController&  beam()    { return beam_;    }
    StageController& stage()   { return stage_;   }
    VacuumSystem&    vacuum()  { return vacuum_;  }

    // High-level commands
    bool set_magnification(float mag);
    bool set_scan_mode(ScanMode mode);
    bool acquire_image();
    bool emergency_stop();
    bool reset();

    SystemState    state()          const;
    ImagingParams  imaging_params() const;
    std::string    full_status()    const;

    static constexpr float MAG_MIN =     10.0f;
    static constexpr float MAG_MAX = 500000.0f;

private:
    void set_state(SystemState s);

    BeamController   beam_;
    StageController  stage_;
    VacuumSystem     vacuum_;
    ImagingParams    imaging_;
    mutable std::mutex mu_;
    SystemState        state_{SystemState::IDLE};
};

} // namespace em
