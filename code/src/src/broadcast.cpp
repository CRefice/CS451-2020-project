#include "broadcast.hpp"

namespace msg {
Broadcast::Broadcast(Parser& parser, udp::Socket& socket, Observer& observer)
    : observer(observer) {
  const auto process_id = parser.id();
  for (auto& host : parser.hosts()) {
    links.emplace_back(process_id, host, socket, *this);
  }
}

void Broadcast::send(const Message& msg) {
  for (auto& link : links) {
    link.send(msg);
  }
}

void Broadcast::deliver(const Message& msg) { observer.deliver(msg); }

void Broadcast::receive(const Message& msg) {
  auto idx = msg.link_id.sender - 1;
  links[idx].deliver(msg);
}

void Broadcast::resynchronize() {
  for (auto& link : links) {
    link.resynchronize();
  }
}

void UniformReliableBroadcast::send(int broadcast_num) {
  auto broadcast_id = Identifier{process_id, broadcast_num};
  Message msg{};
  msg.broadcast_id = broadcast_id;
  bc.send(msg);
}

bool UniformReliableBroadcast::can_deliver(const Message& msg) {
  return ack[msg.broadcast_id].size() > (num_processes / 2) &&
         pending.find(msg.broadcast_id) != pending.end() &&
         delivered.find(msg.broadcast_id) == delivered.end();
}

void UniformReliableBroadcast::deliver(const Message& msg) {
  ack[msg.broadcast_id].insert(msg.link_id.sender);
  if (can_deliver(msg)) {
    delivered.insert(msg.broadcast_id);
    observer.deliver(msg);
  }
  if (pending.find(msg.broadcast_id) == pending.end()) {
    pending.insert(msg.broadcast_id);
    if (can_deliver(msg)) {
      delivered.insert(msg.broadcast_id);
      observer.deliver(msg);
    }
    bc.send(msg);
  }
}
} // namespace msg
