#include "uniform-reliable-broadcast.hpp"

namespace msg {
static void insert(std::vector<RollingBitset>& set, const msg::Message& msg) {
  set[msg.originator - 1].insert(msg.bcast_seq_num - 1);
}

static bool contains(std::vector<RollingBitset>& set, const msg::Message& msg) {
  return set[msg.originator - 1].contains(msg.bcast_seq_num - 1);
}

UniformReliableBroadcast::UniformReliableBroadcast(Parser& parser,
                                                   FairLossLink& link,
                                                   Observer& observer)
    : id(static_cast<ProcessId>(parser.id())),
      num_processes(static_cast<std::uint8_t>(parser.hosts().size())),
      pending(num_processes), delivered(num_processes), bc(parser, link, *this),
      observer(observer) {}

void UniformReliableBroadcast::send(BroadcastSeqNum seq_num) {
  Message msg{};
  msg.originator = id;
  msg.bcast_seq_num = seq_num;
  insert(pending, msg);
  bc.send(msg);
}

bool UniformReliableBroadcast::can_deliver(const Message& msg) {
  return ack[msg].count() > (num_processes / 2) && contains(pending, msg) &&
         !contains(delivered, msg);
}

void UniformReliableBroadcast::try_deliver(const Message& msg) {
  if (can_deliver(msg)) {
    insert(delivered, msg);
    observer.deliver(msg);
  }
}

void UniformReliableBroadcast::deliver(const Message& msg) {
  ack[msg].set(msg.sender - 1);
  if (!contains(pending, msg)) {
    bc.send(msg);
  }
  insert(pending, msg);
  try_deliver(msg);
}
} // namespace msg
