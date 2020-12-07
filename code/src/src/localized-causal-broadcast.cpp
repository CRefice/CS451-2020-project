#include "localized-causal-broadcast.hpp"

namespace msg {
static void print_vector_clock(const VectorClock& c) {
  for (const auto& item : c) {
    std::cerr << item << ',';
  }
  std::cerr << '\n';
}

LocalizedCausalBroadcast::LocalizedCausalBroadcast(ConfigParser& config,
                                                   Parser& parser,
                                                   FairLossLink& link,
                                                   Observer& observer)
    : id(static_cast<ProcessId>(parser.id())), deps(config.dependencies()),
      current_clock(parser.hosts().size()), bc(parser, link, *this),
      observer(observer) {}

void LocalizedCausalBroadcast::send(BroadcastSeqNum sequence_num) {
  Message msg{};

  std::cerr << "V=";
  print_vector_clock(current_clock);

  const auto& my_deps = deps[id - 1];
  for (ProcessId process : my_deps.depends) {
    msg.vector_clock.push_back(current_clock[process - 1]);
  }

  std::cerr << "sent " << sequence_num << " with W=";
  print_vector_clock(msg.vector_clock);

  bc.send(sequence_num, msg);
}

bool LocalizedCausalBroadcast::can_deliver(const Message& msg) {
  if (msg.bcast_seq_num - 1 > current_clock[msg.originator - 1]) {
    return false;
  }

  const auto& depends = deps[msg.originator - 1].depends;
  for (auto i = 0ul; i < msg.vector_clock.size(); ++i) {
    auto process_id = depends[i];
    if (msg.vector_clock[i] > current_clock[process_id - 1]) {
      return false;
    }
  }
  return true;
}

void LocalizedCausalBroadcast::deliver(const Message& msg) {
  std::cerr << "Got " << msg.bcast_seq_num << " from " << +msg.originator
            << " with W= ";
  print_vector_clock(msg.vector_clock);

  pending.push_back(msg);

  for (auto it = pending.begin(); it != pending.end();) {
    const auto& to_deliver = *it;
    if (can_deliver(to_deliver)) {
      std::cerr << "Delivered " << to_deliver.bcast_seq_num << " from "
                << +to_deliver.originator << " with W= ";
      print_vector_clock(to_deliver.vector_clock);

      current_clock[to_deliver.originator - 1]++;
      observer.deliver(to_deliver);
      pending.erase(it);
      it = pending.begin();
    } else {
      ++it;
    }
  }
}
} // namespace msg
