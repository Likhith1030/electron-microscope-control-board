#include "control_board/MessageParser.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace em;

static int passed = 0;
static int failed = 0;

#define ASSERT(cond, name) \
    do { \
        if (cond) { std::cout << "  PASS: " name "\n"; ++passed; } \
        else       { std::cout << "  FAIL: " name "\n"; ++failed; } \
    } while(0)

void test_round_trip() {
    std::cout << "\n[test_round_trip]\n";

    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = CommandId::SET_BEAM_VOLTAGE;
    push_f32(f.payload, 15.5f);

    auto wire = f.serialize();

    std::vector<Frame> received;
    MessageParser parser([&](const Frame& fr) { received.push_back(fr); });
    parser.feed(wire);

    ASSERT(received.size() == 1,               "one frame delivered");
    ASSERT(received[0].type == f.type,         "type preserved");
    ASSERT(received[0].cmd  == f.cmd,          "cmd preserved");
    ASSERT(received[0].payload.size() == 4,    "payload length");
    float v = pop_f32(received[0].payload.data());
    ASSERT(v == 15.5f,                         "float value preserved");
    ASSERT(parser.frames_parsed()   == 1,      "parsed count");
    ASSERT(parser.frames_rejected() == 0,      "rejected count");
}

void test_crc_rejection() {
    std::cout << "\n[test_crc_rejection]\n";

    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = CommandId::SET_FOCUS;
    push_f32(f.payload, 10.0f);

    auto wire = f.serialize();
    wire[wire.size() - 1] ^= 0xFF;   // corrupt last CRC byte

    std::vector<Frame> received;
    MessageParser parser([&](const Frame& fr) { received.push_back(fr); });
    parser.feed(wire);

    ASSERT(received.empty(),              "no frame delivered");
    ASSERT(parser.frames_rejected() == 1, "rejected count");
}

void test_multi_frame_stream() {
    std::cout << "\n[test_multi_frame_stream]\n";

    std::vector<uint8_t> stream;
    for (uint8_t i = 0; i < 5; ++i) {
        Frame f;
        f.type = MessageType::COMMAND;
        f.cmd  = CommandId::SET_STAGE_X;
        push_f32(f.payload, float(i) * 1000.0f);
        auto w = f.serialize();
        stream.insert(stream.end(), w.begin(), w.end());
    }

    std::vector<Frame> received;
    MessageParser parser([&](const Frame& fr) { received.push_back(fr); });
    parser.feed(stream);

    ASSERT(received.size() == 5,          "five frames parsed");
    ASSERT(parser.frames_parsed() == 5,   "parsed count == 5");
}

void test_empty_payload() {
    std::cout << "\n[test_empty_payload]\n";

    Frame f;
    f.type = MessageType::COMMAND;
    f.cmd  = CommandId::ENABLE_BEAM;
    // no payload

    auto wire = f.serialize();
    std::vector<Frame> received;
    MessageParser parser([&](const Frame& fr) { received.push_back(fr); });
    parser.feed(wire);

    ASSERT(received.size() == 1,         "one frame");
    ASSERT(received[0].payload.empty(),  "empty payload");
}

void test_byte_by_byte_feed() {
    std::cout << "\n[test_byte_by_byte_feed]\n";

    Frame f;
    f.type = MessageType::RESPONSE;
    f.cmd  = CommandId::GET_STATUS;
    push_u8(f.payload, uint8_t(ResponseCode::OK));
    push_u8(f.payload, uint8_t(SystemState::READY));

    auto wire = f.serialize();
    std::vector<Frame> received;
    MessageParser parser([&](const Frame& fr) { received.push_back(fr); });
    for (uint8_t byte : wire) parser.feed(byte);

    ASSERT(received.size() == 1,  "frame assembled byte-by-byte");
}

int main() {
    std::cout << "=== MessageParser Tests ===\n";
    test_round_trip();
    test_crc_rejection();
    test_multi_frame_stream();
    test_empty_payload();
    test_byte_by_byte_feed();

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
