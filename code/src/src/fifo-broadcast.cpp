#include "fifo-broadcast.hpp"

namespace msg {
FifoBroadcast::FifoBroadcast(Parser& parser, FairLossLink& link,
                             Observer& observer)
    : ordered(parser.hosts().size()), bc(parser, link, *this),
      observer(observer) {}

void FifoBroadcast::send(BroadcastSeqNum sequence_num) {
  auto msg = Message{};
  bc.send(sequence_num, msg);
}

void FifoBroadcast::deliver(const Message& msg) {
  auto& queue = ordered[msg.originator];
  queue.add(msg, msg.bcast_seq_num);
  while (queue.has_next()) {
    auto item = queue.pop_next();
    observer.deliver(item);
  }
}
} // namespace msg
