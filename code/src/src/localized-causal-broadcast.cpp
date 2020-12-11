#include "localized-causal-broadcast.hpp"

namespace msg {
LocalizedCausalBroadcast::LocalizedCausalBroadcast(ConfigParser& config,
                                                   Parser& parser,
                                                   FairLossLink& link,
                                                   Observer& observer)
    : id(parser.index()), deps(config.dependencies()),
      pending(parser.hosts().size()), current_clock(pending.size()),
      bc(parser, link, *this), observer(observer) {}

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
  pending[msg.originator].push(msg);
  for (auto it = pending.begin(); it != pending.end();) {
    auto& queue = *it;
    if (!queue.empty() && can_deliver(queue.top())) {
      const auto& msg = queue.top();
      current_clock[msg.originator]++;
      observer.deliver(msg);
      queue.pop();
      it = pending.begin();
    } else {
      ++it;
    }
  }
}
} // namespace msg
