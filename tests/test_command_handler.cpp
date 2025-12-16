#include "control_board/CommandHandler.hpp"
#include "microscope/MicroscopeController.hpp"
#include <cassert>
#include <iostream>

using namespace em;

static int passed = 0;
static int failed = 0;

#define ASSERT(cond, name) \
    do { \
        if (cond) { std::cout << "  PASS: " name "\n"; ++passed; } \
        else       { std::cout << "  FAIL: " name "\n"; ++failed; } \
    } while(0)

static Frame make_f32_cmd(CommandId id, float val) {
    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = id;
    push_f32(f.payload, val);
    return f;
}
static Frame make_empty_cmd(CommandId id) {
    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = id;
    return f;
}
static ResponseCode rc(const Frame& resp) {
    return static_cast<ResponseCode>(resp.payload[0]);
}

void test_beam_commands() {
    std::cout << "\n[test_beam_commands]\n";
    MicroscopeController scope;
    CommandHandler handler(scope);

    auto r1 = handler.handle(make_f32_cmd(CommandId::SET_BEAM_VOLTAGE, 10.0f));
    ASSERT(rc(r1) == ResponseCode::OK, "set voltage 10 kV -> OK");

    auto r2 = handler.handle(make_f32_cmd(CommandId::SET_BEAM_VOLTAGE, 99.0f));
    ASSERT(rc(r2) == ResponseCode::ERR_OUT_OF_RANGE, "set voltage 99 kV -> range error");

    auto r3 = handler.handle(make_f32_cmd(CommandId::SET_BEAM_CURRENT, 5.0f));
    ASSERT(rc(r3) == ResponseCode::OK, "set current 5 nA -> OK");

    auto r4 = handler.handle(make_empty_cmd(CommandId::ENABLE_BEAM));
    ASSERT(rc(r4) == ResponseCode::OK, "enable beam -> OK (vacuum ready)");

    auto r5 = handler.handle(make_empty_cmd(CommandId::DISABLE_BEAM));
    ASSERT(rc(r5) == ResponseCode::OK, "disable beam -> OK");
}

void test_stage_commands() {
    std::cout << "\n[test_stage_commands]\n";
    MicroscopeController scope;
    CommandHandler handler(scope);

    ASSERT(rc(handler.handle(make_f32_cmd(CommandId::SET_STAGE_X, 1000.0f))) == ResponseCode::OK,
           "X = 1000 µm -> OK");
    ASSERT(rc(handler.handle(make_f32_cmd(CommandId::SET_STAGE_X, 999999.0f))) == ResponseCode::ERR_OUT_OF_RANGE,
           "X = 999999 µm -> range error");
    ASSERT(rc(handler.handle(make_f32_cmd(CommandId::SET_STAGE_Y, -3000.0f))) == ResponseCode::OK,
           "Y = -3000 µm -> OK");
    ASSERT(rc(handler.handle(make_f32_cmd(CommandId::SET_STAGE_Z, 5000.0f))) == ResponseCode::OK,
           "Z = 5000 µm -> OK");
    ASSERT(rc(handler.handle(make_f32_cmd(CommandId::SET_STAGE_TILT, 80.0f))) == ResponseCode::ERR_OUT_OF_RANGE,
           "Tilt = 80° -> range error");
}

void test_emergency_stop() {
    std::cout << "\n[test_emergency_stop]\n";
    MicroscopeController scope;
    CommandHandler handler(scope);

    handler.handle(make_f32_cmd(CommandId::SET_BEAM_VOLTAGE, 10.0f));
    handler.handle(make_empty_cmd(CommandId::ENABLE_BEAM));

    auto r_stop = handler.handle(make_empty_cmd(CommandId::EMERGENCY_STOP));
    ASSERT(rc(r_stop) == ResponseCode::OK, "emergency stop -> OK");
    ASSERT(scope.state() == SystemState::EMERGENCY, "state is EMERGENCY");
    ASSERT(!scope.beam().params().beam_enabled,     "beam disabled after e-stop");

    auto r_acq = handler.handle(make_empty_cmd(CommandId::ACQUIRE_IMAGE));
    ASSERT(rc(r_acq) == ResponseCode::ERR_INVALID_STATE, "acquire in EMERGENCY -> error");

    auto r_rst = handler.handle(make_empty_cmd(CommandId::RESET));
    ASSERT(rc(r_rst) == ResponseCode::OK, "reset -> OK");
    ASSERT(scope.state() == SystemState::READY, "state is READY after reset");
}

void test_imaging_flow() {
    std::cout << "\n[test_imaging_flow]\n";
    MicroscopeController scope;
    CommandHandler handler(scope);

    handler.handle(make_f32_cmd(CommandId::SET_BEAM_VOLTAGE, 20.0f));
    handler.handle(make_empty_cmd(CommandId::ENABLE_BEAM));
    handler.handle(make_f32_cmd(CommandId::SET_MAGNIFICATION, 10000.0f));

    ASSERT(rc(handler.handle(make_empty_cmd(CommandId::ACQUIRE_IMAGE))) == ResponseCode::OK,
           "acquire 1 -> OK");
    ASSERT(rc(handler.handle(make_empty_cmd(CommandId::ACQUIRE_IMAGE))) == ResponseCode::OK,
           "acquire 2 -> OK");
    ASSERT(scope.imaging_params().frame_count == 2, "frame count == 2");
}

int main() {
    std::cout << "=== CommandHandler Tests ===\n";
    test_beam_commands();
    test_stage_commands();
    test_emergency_stop();
    test_imaging_flow();

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
