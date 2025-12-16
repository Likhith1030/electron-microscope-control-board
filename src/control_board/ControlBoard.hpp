#pragma once

#include "control_board/MessageParser.hpp"
#include "control_board/CommandHandler.hpp"
#include "host/CommunicationChannel.hpp"
#include "microscope/MicroscopeController.hpp"
#include <thread>
#include <atomic>

namespace em {

// Simulates the firmware running on the electronics control board.
// Spawns a background thread that reads from the channel, parses frames,
// dispatches commands, and writes response frames back.
class ControlBoard {
public:
    explicit ControlBoard(CommunicationChannel& ch);
    ~ControlBoard();

    // Start/stop the processing loop.
    void start();
    void stop();

    bool is_running() const { return running_; }

    // Direct access for test inspection
    const MicroscopeController& scope() const { return scope_; }
    const MessageParser&        parser() const { return parser_; }

private:
    void run();

    CommunicationChannel& ch_;
    MicroscopeController  scope_;
    MessageParser         parser_;
    CommandHandler        handler_;
    std::thread           thread_;
    std::atomic<bool>     running_{false};
};

} // namespace em
