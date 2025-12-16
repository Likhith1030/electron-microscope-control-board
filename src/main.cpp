#include "control_board/ControlBoard.hpp"
#include "host/CommunicationChannel.hpp"
#include "host/HostSystem.hpp"
#include "common/Logger.hpp"

#include <iostream>
#include <thread>
#include <chrono>

using namespace em;
using namespace std::chrono_literals;

static void check(const char* label, std::optional<ResponseCode> r) {
    if (!r) {
        std::cout << "  [TIMEOUT] " << label << "\n";
        return;
    }
    const char* status = (*r == ResponseCode::OK) ? "OK" : "FAIL";
    std::cout << "  [" << status << "] " << label;
    if (*r != ResponseCode::OK)
        std::cout << " (code=0x" << std::hex << int(*r) << std::dec << ")";
    std::cout << "\n";
}

int main() {
    Logger::instance().set_level(LogLevel::INFO);

    std::cout << "\n========================================\n";
    std::cout << "  Electron Microscope Control Board\n";
    std::cout << "  Interface Simulator\n";
    std::cout << "========================================\n\n";

    // ── Setup ────────────────────────────────────────────────────────────────
    CommunicationChannel channel;
    ControlBoard board(channel);
    HostSystem   host(channel, 500ms);

    board.start();
    std::this_thread::sleep_for(100ms); // let board thread stabilise

    // ── Scenario 1: normal imaging session ───────────────────────────────────
    std::cout << "\n--- Scenario 1: Normal Imaging Session ---\n";

    std::cout << "\n[1] Beam setup\n";
    check("Set voltage 10 kV",    host.set_beam_voltage(10.0f));
    check("Set current 5 nA",     host.set_beam_current(5.0f));
    check("Set focus 8.5 mm",     host.set_focus(8.5f));
    check("Enable beam",          host.enable_beam());

    std::cout << "\n[2] Stage positioning\n";
    check("Move X to 1500 µm",    host.set_stage_x(1500.0f));
    check("Move Y to -2000 µm",   host.set_stage_y(-2000.0f));
    check("Move Z to 10000 µm",   host.set_stage_z(10000.0f));
    check("Tilt to 15°",          host.set_stage_tilt(15.0f));

    std::cout << "\n[3] Imaging\n";
    check("Set magnification 5000x",  host.set_magnification(5000.0f));
    check("Set scan mode RASTER",     host.set_scan_mode(ScanMode::RASTER));
    check("Acquire frame 1",          host.acquire_image());
    check("Acquire frame 2",          host.acquire_image());
    check("Acquire frame 3",          host.acquire_image());

    // ── Scenario 2: out-of-range rejection ───────────────────────────────────
    std::cout << "\n--- Scenario 2: Out-of-Range Rejection ---\n";
    check("Voltage 99 kV (too high)",  host.set_beam_voltage(99.0f));
    check("Stage X 999999 µm",         host.set_stage_x(999999.0f));
    check("Tilt 90° (over limit)",      host.set_stage_tilt(90.0f));
    check("Mag 0x (too low)",           host.set_magnification(0.0f));

    // ── Scenario 3: emergency stop & reset ───────────────────────────────────
    std::cout << "\n--- Scenario 3: Emergency Stop & Reset ---\n";
    check("Emergency stop",             host.emergency_stop());
    check("Acquire after e-stop (fail)",host.acquire_image());
    check("Reset",                      host.reset());
    check("Enable beam after reset",    host.enable_beam());
    check("Acquire after reset",        host.acquire_image());

    // ── Scenario 4: telemetry queries ────────────────────────────────────────
    std::cout << "\n--- Scenario 4: Status Queries ---\n";
    check("Get system status",          host.get_status());
    check("Get vacuum status",          host.get_vacuum_status());

    // ── Final state ──────────────────────────────────────────────────────────
    std::cout << "\n--- Final Microscope State ---\n";
    std::cout << board.scope().full_status() << "\n";

    std::cout << "\n--- Parser Statistics ---\n";
    std::cout << "  Frames parsed:   " << board.parser().frames_parsed()   << "\n";
    std::cout << "  Frames rejected: " << board.parser().frames_rejected()  << "\n";

    board.stop();
    std::cout << "\nSimulation complete.\n\n";
    return 0;
}
