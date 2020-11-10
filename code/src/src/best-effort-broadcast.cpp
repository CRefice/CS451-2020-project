#include "best-effort-broadcast.hpp"

#include <iostream>

namespace msg {
BestEffortBroadcast::BestEffortBroadcast(Parser& parser, FairLossLink& link,
                                         Observer& observer)
    : id(static_cast<ProcessId>(parser.id())), link(parser, link, *this),
      observer(observer) {
  const auto n = static_cast<ProcessId>(parser.hosts().size());
  if (n == 1) {
    return;
  }
  if (n == 2) {
    connections.push_back(static_cast<ProcessId>(n - id + 1));
    return;
  }
  create_n_connected_ring(n);
}

void BestEffortBroadcast::send(Message msg) {
  for (auto neighbor : connections) {
    link.send(neighbor, msg);
  }
  // Also send to self
  link.send(id, msg);
}

void BestEffortBroadcast::deliver(const Message& msg) { observer.deliver(msg); }

void BestEffortBroadcast::create_n_connected_ring(ProcessId n) {
  const auto neighbors_per_side =
      static_cast<ProcessId>((n + 3) / 4); // Ceiling of division by 4
  connections.reserve(neighbors_per_side * 2);
  for (ProcessId offset = 1; offset <= neighbors_per_side; ++offset) {
    const auto right_neighbor = (id - 1 + offset) % n + 1;
    const auto left_neighbor = (id - 1 + n - offset) % n + 1;
    connections.emplace_back(right_neighbor);
    connections.emplace_back(left_neighbor);
  }
}
} // namespace msg
