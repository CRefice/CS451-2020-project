#include "broadcast.hpp"

namespace msg {
Broadcast::Broadcast(Parser& parser, udp::Socket& socket, Observer& observer)
    : observer(observer) {
  const auto process_id = parser.id();
  for (auto& host : parser.hosts()) {
    links.emplace_back(process_id, host, socket, *this);
  }
}

void Broadcast::send(int broadcast_num) {
  for (auto& link : links) {
    link.send(broadcast_num);
  }
}

void Broadcast::deliver(const Message& msg) { observer.deliver(msg); }

void Broadcast::receive(const Message& msg) {
  auto idx = msg.id.sender - 1;
  links[idx].deliver(msg);
}

void Broadcast::resynchronize() {
  for (auto& link : links) {
    link.resynchronize();
  }
}
} // namespace msg
