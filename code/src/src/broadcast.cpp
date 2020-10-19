#include "broadcast.hpp"

namespace msg {
BestEffortBroadcast::BestEffortBroadcast(Parser& parser, FairLossLink& link,
                                         Observer& observer)
    : process_id(static_cast<unsigned int>(parser.id())),
      num_processes(parser.hosts().size()), link(parser, link, *this),
      observer(observer) {}

void BestEffortBroadcast::send(Message msg) {
  for (auto p = 1u; p <= num_processes; ++p) {
    link.send(p, msg);
  }
}

void BestEffortBroadcast::deliver(const Message& msg) { observer.deliver(msg); }

UniformReliableBroadcast::UniformReliableBroadcast(Parser& parser,
                                                   FairLossLink& link,
                                                   Observer& observer)
    : process_id(static_cast<unsigned int>(parser.id())),
      num_processes(parser.hosts().size()), delivered(num_processes),
      bc(parser, link, *this), observer(observer) {}

void UniformReliableBroadcast::send(unsigned int sequence_num) {
  Message msg{};
  msg.broadcast_id.sender = process_id;
  msg.broadcast_id.sequence_num = sequence_num;
  pending.insert(msg.broadcast_id);
  bc.send(msg);
}

bool UniformReliableBroadcast::can_deliver(const Message& msg) {
  return ack[msg.broadcast_id].count() > (num_processes / 2) &&
         pending.find(msg.broadcast_id) != pending.end() &&
         delivered.find(msg.broadcast_id) == delivered.end();
}

void UniformReliableBroadcast::try_deliver(const Message& msg) {
  if (can_deliver(msg)) {
    delivered.insert(msg.broadcast_id);
    observer.deliver(msg);
  }
}

void UniformReliableBroadcast::deliver(const Message& msg) {
  ack[msg.broadcast_id].set(msg.link_id.sender - 1, true);
  try_deliver(msg);
  if (pending.find(msg.broadcast_id) == pending.end()) {
    pending.insert(msg.broadcast_id);
    try_deliver(msg);
    bc.send(msg);
  }
}

FifoBroadcast::FifoBroadcast(Parser& parser, FairLossLink& link,
                             Observer& observer)
    : ordered(parser.hosts().size()), bc(parser, link, *this),
      observer(observer) {}

void FifoBroadcast::send(unsigned int sequence_num) { bc.send(sequence_num); }

void FifoBroadcast::deliver(const Message& msg) {
  auto& queue = ordered[msg.broadcast_id.sender - 1];
  queue.add(msg, msg.broadcast_id.sequence_num - 1);
  while (queue.has_next()) {
    observer.deliver(queue.pop_next());
  }
}
} // namespace msg
