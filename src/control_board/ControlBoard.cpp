#include "control_board/ControlBoard.hpp"
#include "common/Logger.hpp"

namespace em {

ControlBoard::ControlBoard(CommunicationChannel& ch)
    : ch_(ch),
      scope_(),
      parser_([this](const Frame& frame) {
          // Called on the board's processing thread each time a complete frame arrives.
          Frame response = handler_.handle(frame);
          ch_.board_to_host().write(response.serialize());
      }),
      handler_(scope_) {}

ControlBoard::~ControlBoard() { stop(); }

void ControlBoard::start() {
    running_ = true;
    thread_  = std::thread(&ControlBoard::run, this);
    LOG_INFO("ControlBoard", "Processing loop started");
}

void ControlBoard::stop() {
    if (running_.exchange(false) && thread_.joinable()) {
        thread_.join();
        LOG_INFO("ControlBoard", "Processing loop stopped");
    }
}

void ControlBoard::run() {
    using namespace std::chrono_literals;
    while (running_) {
        auto pkt = ch_.host_to_board().read(50ms);
        if (pkt) parser_.feed(*pkt);
    }
}

} // namespace em
