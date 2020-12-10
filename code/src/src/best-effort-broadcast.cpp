#include "best-effort-broadcast.hpp"

#include <iostream>

namespace msg {
static std::vector<ProcessId> create_n_connected_ring(ProcessId id,
                                                      std::size_t n) {
  if (n == 1) {
    return {};
  }
  if (n == 2) {
    auto other = 1 - id;
    return {static_cast<ProcessId>(other)};
  }

  std::vector<ProcessId> connections;
  const auto neighbors_per_side =
      static_cast<ProcessId>((n + 3) / 4); // Ceiling of division by 4
  connections.reserve(neighbors_per_side * 2);
  for (ProcessId offset = 1; offset <= neighbors_per_side; ++offset) {
    const auto right_neighbor = (id + offset) % n;
    const auto left_neighbor = (id + n - offset) % n;
    connections.emplace_back(right_neighbor);
    connections.emplace_back(left_neighbor);
  }
  return connections;
}

BestEffortBroadcast::BestEffortBroadcast(Parser& parser, FairLossLink& link,
                                         Observer& observer)
    : id(parser.index()),
      connections(create_n_connected_ring(id, parser.hosts().size())),
      link(parser, link, *this), observer(observer) {}

void BestEffortBroadcast::send(Message& msg) {
  for (auto neighbor : connections) {
    link.send(neighbor, msg);
  }
  // Also send to self
  link.send(id, msg);
}

void BestEffortBroadcast::deliver(const Message& msg) { observer.deliver(msg); }
} // namespace msg
