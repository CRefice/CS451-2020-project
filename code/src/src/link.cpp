#include <cassert>

#include "link.hpp"

namespace msg {
Link::Link(Parser& parser, udp::Socket& socket, Observer& observer)
    : process_id(parser.id()), socket(socket), observer(observer) {
  host_addrs.resize(parser.hosts().size());
  for (auto& host : parser.hosts()) {
    address_of(host.id) = udp::socket_address(host.ip, host.port);
  }
}

void Link::do_send(unsigned long receiver, const Message& msg) {
  socket.send(address_of(receiver), reinterpret_cast<const char*>(&msg),
              sizeof(msg));
}

void Link::send(unsigned long receiver, int broadcast_seq_num) {
  const auto id = Identifier{process_id, next_seq_num};
  const auto msg = Message{id, broadcast_seq_num, Syn};
  do_send(receiver, msg);
  sent_msgs.emplace(next_seq_num++, std::make_pair(msg, receiver));
}

void Link::deliver(const Message& msg) {
  if (msg.discr == Syn) {
    // New message incoming. Send acknowledgment
    Message ack(msg);
    ack.discr = Ack;
    do_send(msg.id.sender, ack);
    observer.deliver(msg);
  } else {
    // Received acknowledgement, remove from sent set
    sent_msgs.erase(msg.id.sequence_num);
  }
}

void Link::resynchronize() {
  for (const auto& [_, entry] : sent_msgs) {
    const auto& [msg, receiver] = entry;
    do_send(receiver, msg);
  }
}

void PerfectLink::deliver(const Message& msg) {
  if (delivered_msgs.find(msg.id) == delivered_msgs.end()) {
    delivered_msgs.emplace(msg.id);
    observer.deliver(msg);
  }
}
} // namespace msg
