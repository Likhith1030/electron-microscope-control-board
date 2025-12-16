#pragma once

#include <mutex>
#include <string>

namespace em {

struct BeamParams {
    float voltage_kV   = 0.0f;   // 0.1 – 30.0 kV
    float current_nA   = 0.0f;   // 0.1 – 100.0 nA
    float focus_mm     = 0.0f;   // working distance 1 – 50 mm
    bool  beam_enabled = false;
};

class BeamController {
public:
    // Returns true on success, false if value is out of range.
    bool set_voltage(float kV);
    bool set_current(float nA);
    bool set_focus(float mm);
    bool enable();
    bool disable();

    BeamParams params() const;
    std::string status_string() const;

    // Limits
    static constexpr float VOLTAGE_MIN_KV =  0.1f;
    static constexpr float VOLTAGE_MAX_KV = 30.0f;
    static constexpr float CURRENT_MIN_NA =  0.1f;
    static constexpr float CURRENT_MAX_NA = 100.0f;
    static constexpr float FOCUS_MIN_MM   =  1.0f;
    static constexpr float FOCUS_MAX_MM   = 50.0f;

private:
    mutable std::mutex mu_;
    BeamParams         params_;
};

} // namespace em
