#include "localized-causal-broadcast.hpp"

namespace msg {
LocalizedCausalBroadcast::LocalizedCausalBroadcast(ConfigParser& config,
                                                   Parser& parser,
                                                   FairLossLink& link,
                                                   Observer& observer)
    : id(parser.index()), deps(config.dependencies()),
      current_clock(parser.hosts().size()), bc(parser, link, *this),
      observer(observer) {}

void LocalizedCausalBroadcast::send(BroadcastSeqNum sequence_num) {
  Message msg{};

  const auto& my_deps = deps[id];
  for (ProcessId process : my_deps.depends) {
    msg.vector_clock.push_back(current_clock[process]);
  }

  bc.send(sequence_num, msg);
}

bool LocalizedCausalBroadcast::can_deliver(const Message& msg) {
  if (msg.bcast_seq_num > current_clock[msg.originator]) {
    return false;
  }

  const auto& depends = deps[msg.originator].depends;
  for (auto i = 0ul; i < msg.vector_clock.size(); ++i) {
    auto process_id = depends[i];
    if (msg.vector_clock[i] > current_clock[process_id]) {
      return false;
    }
  }
  return true;
}

void LocalizedCausalBroadcast::deliver(const Message& msg) {
  pending.push_back(msg);
  for (auto it = pending.begin(); it != pending.end();) {
    auto& to_deliver = *it;
    if (can_deliver(to_deliver)) {
      current_clock[to_deliver.originator]++;
      observer.deliver(to_deliver);
      std::swap(to_deliver, pending.back());
      pending.pop_back();
      it = pending.begin();
    } else {
      ++it;
    }
  }
}
} // namespace msg
