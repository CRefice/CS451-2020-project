#include "best-effort-broadcast.hpp"

namespace msg {
BestEffortBroadcast::BestEffortBroadcast(Parser& parser, FairLossLink& link,
                                         Observer& observer)
    : num_processes(static_cast<std::uint8_t>(parser.hosts().size())),
      link(parser, link, *this), observer(observer) {}

void BestEffortBroadcast::send(Message msg) {
  for (std::uint8_t p = 1; p <= num_processes; ++p) {
    link.send(p, msg);
  }
}

void BestEffortBroadcast::deliver(const Message& msg) { observer.deliver(msg); }
} // namespace msg
