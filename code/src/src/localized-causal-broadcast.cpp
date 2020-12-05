#include "localized-causal-broadcast.hpp"

namespace msg {
LocalizedCausalBroadcast::LocalizedCausalBroadcast(ConfigParser& config,
                                                   Parser& parser,
                                                   FairLossLink& link,
                                                   Observer& observer)
    : id(static_cast<ProcessId>(parser.id())), deps(config.dependencies()),
      current_clock(parser.hosts().size()), bc(parser, link, *this),
      observer(observer) {}

void LocalizedCausalBroadcast::send(BroadcastSeqNum sequence_num) {
  Message msg{};
  msg.vector_clock = current_clock;
  msg.vector_clock[id - 1] = lsn++;

  std::cerr << "Broadcasting " << sequence_num << " with V= ";
  for (auto v : msg.vector_clock) {
    std::cerr << v << ',';
  }
  std::cerr << '\n';
  bc.send(sequence_num, msg);
}

bool LocalizedCausalBroadcast::can_deliver(const Message& msg) {
  for (auto i = 0ul; i < current_clock.size(); ++i) {
    if (msg.vector_clock[i] > current_clock[i]) {
      return false;
    }
  }
  return true;
}

// TODO: actually localize
void LocalizedCausalBroadcast::deliver(const Message& msg) {
  std::cerr << "Got " << msg.bcast_seq_num << " from " << +msg.originator
            << " with W= ";
  for (auto v : msg.vector_clock) {
    std::cerr << v << ',';
  }
  std::cerr << '\n';

  pending.push_back(msg);

  for (auto it = pending.begin(); it != pending.end();) {
    const auto& to_deliver = *it;
    if (can_deliver(to_deliver)) {
      current_clock[to_deliver.originator - 1]++;
      observer.deliver(to_deliver);
      pending.erase(it);
      it = pending.begin();
    } else {
      ++it;
    }
    // std::cerr << "Top msg " << to_deliver.bcast_seq_num << " from "
    //          << to_deliver.originator << " out of " << pending.size() <<
    //          '\n';
  }
}
} // namespace msg
