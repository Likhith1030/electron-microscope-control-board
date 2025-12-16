#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>

namespace em {

// Thread-safe byte-stream channel with two directions.
// Host writes to host_to_board(); board reads from host_to_board().
// Board writes to board_to_host(); host reads from board_to_host().
class CommunicationChannel {
public:
    // One logical pipe (FIFO of byte packets)
    class Pipe {
    public:
        void write(const std::vector<uint8_t>& data);
        // Block until data arrives or timeout elapses.  Returns nullopt on timeout.
        std::optional<std::vector<uint8_t>> read(std::chrono::milliseconds timeout);
        // Non-blocking try-read.
        std::optional<std::vector<uint8_t>> try_read();

    private:
        std::queue<std::vector<uint8_t>> queue_;
        std::mutex                       mu_;
        std::condition_variable          cv_;
    };

    Pipe& host_to_board() { return h2b_; }
    Pipe& board_to_host() { return b2h_; }

private:
    Pipe h2b_;
    Pipe b2h_;
};

} // namespace em
