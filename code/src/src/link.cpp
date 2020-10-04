#include <cassert>

#include "link.hpp"

namespace msg {
void PerfectFifoLink::do_send(const Message& msg) {
  socket.send(receiver_addr, reinterpret_cast<const char*>(&msg), sizeof(msg));
}

void PerfectFifoLink::send(Message msg) {
  msg.link_id = Identifier{process_id, next_seq_num};
  do_send(msg);
  sent_msgs.emplace(next_seq_num++, msg);
}

void PerfectFifoLink::deliver(const Message& msg) {
  if (msg.discr == Syn) {
    // new message incoming
    const auto seq_num = msg.link_id.sequence_num;
    bool should_ack = true;
    if (!delivered.has_seen(seq_num)) {
      // if we haven't received the message before,
      // try to add it to the received queue.
      // if the queue is full, we do not acknowledge receipt,
      // so the sender will try again.
      should_ack = delivered.add(
          msg, seq_num, [this](const Message& msg) { observer.deliver(msg); });
    }
    Message ack(msg);
    ack.discr = Ack;
    do_send(ack);
  } else {
    // received acknowledgement, remove from sent set
    sent_msgs.erase(msg.link_id.sequence_num);
  }
}

void PerfectFifoLink::resynchronize() {
  for (const auto& [_, msg] : sent_msgs) {
    do_send(msg);
  }
}
} // namespace msg
