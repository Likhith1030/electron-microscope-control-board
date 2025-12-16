#pragma once

#include <mutex>
#include <string>

namespace em {

struct StageParams {
    float x_um    = 0.0f;   // ±50 000 µm
    float y_um    = 0.0f;   // ±50 000 µm
    float z_um    = 0.0f;   //  0 – 40 000 µm (working distance)
    float tilt_deg= 0.0f;   // ±70 °
};

class StageController {
public:
    bool set_x(float um);
    bool set_y(float um);
    bool set_z(float um);
    bool set_tilt(float deg);

    StageParams params() const;
    std::string status_string() const;

    static constexpr float XY_LIMIT_UM   = 50000.0f;
    static constexpr float Z_MIN_UM      =     0.0f;
    static constexpr float Z_MAX_UM      = 40000.0f;
    static constexpr float TILT_LIMIT_DEG=    70.0f;

private:
    mutable std::mutex mu_;
    StageParams        params_;
};

} // namespace em
