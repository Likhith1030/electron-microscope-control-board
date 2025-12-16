#include "control_board/ControlBoard.hpp"
#include "host/CommunicationChannel.hpp"
#include "host/HostSystem.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

using namespace em;
using namespace std::chrono_literals;

static int passed = 0;
static int failed = 0;

#define ASSERT(cond, name) \
    do { \
        if (cond) { std::cout << "  PASS: " name "\n"; ++passed; } \
        else       { std::cout << "  FAIL: " name "\n"; ++failed; } \
    } while(0)

struct Fixture {
    CommunicationChannel channel;
    ControlBoard         board{channel};
    HostSystem           host{channel, 300ms};

    Fixture() {
        board.start();
        std::this_thread::sleep_for(50ms);
    }
    ~Fixture() { board.stop(); }
};

void test_full_session() {
    std::cout << "\n[test_full_session]\n";
    Fixture fx;

    auto r1 = fx.host.set_beam_voltage(15.0f);
    ASSERT(r1 && *r1 == ResponseCode::OK, "set voltage over channel");

    auto r2 = fx.host.set_beam_current(10.0f);
    ASSERT(r2 && *r2 == ResponseCode::OK, "set current over channel");

    auto r3 = fx.host.enable_beam();
    ASSERT(r3 && *r3 == ResponseCode::OK, "enable beam over channel");

    auto r4 = fx.host.set_stage_x(500.0f);
    ASSERT(r4 && *r4 == ResponseCode::OK, "set stage X");

    auto r5 = fx.host.acquire_image();
    ASSERT(r5 && *r5 == ResponseCode::OK, "acquire image");

    ASSERT(fx.board.parser().frames_parsed() >= 5, "at least 5 frames parsed by board");
}

void test_bad_value_rejected() {
    std::cout << "\n[test_bad_value_rejected]\n";
    Fixture fx;

    auto r = fx.host.set_beam_voltage(500.0f);
    ASSERT(r && *r == ResponseCode::ERR_OUT_OF_RANGE, "500 kV rejected end-to-end");
}

void test_emergency_round_trip() {
    std::cout << "\n[test_emergency_round_trip]\n";
    Fixture fx;

    fx.host.set_beam_voltage(10.0f);
    fx.host.enable_beam();

    auto rs = fx.host.emergency_stop();
    ASSERT(rs && *rs == ResponseCode::OK, "e-stop round-trip OK");

    auto ra = fx.host.acquire_image();
    ASSERT(ra && *ra == ResponseCode::ERR_INVALID_STATE, "acquire blocked after e-stop");

    auto rr = fx.host.reset();
    ASSERT(rr && *rr == ResponseCode::OK, "reset round-trip OK");
}

int main() {
    std::cout << "=== Integration Tests (ControlBoard <-> HostSystem) ===\n";
    test_full_session();
    test_bad_value_rejected();
    test_emergency_round_trip();

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
