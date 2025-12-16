#include "host/CommunicationChannel.hpp"

namespace em {

void CommunicationChannel::Pipe::write(const std::vector<uint8_t>& data) {
    {
        std::lock_guard<std::mutex> lk(mu_);
        queue_.push(data);
    }
    cv_.notify_one();
}

std::optional<std::vector<uint8_t>>
CommunicationChannel::Pipe::read(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lk(mu_);
    if (!cv_.wait_for(lk, timeout, [this]{ return !queue_.empty(); }))
        return std::nullopt;
    auto pkt = std::move(queue_.front());
    queue_.pop();
    return pkt;
}

std::optional<std::vector<uint8_t>> CommunicationChannel::Pipe::try_read() {
    std::lock_guard<std::mutex> lk(mu_);
    if (queue_.empty()) return std::nullopt;
    auto pkt = std::move(queue_.front());
    queue_.pop();
    return pkt;
}

} // namespace em
