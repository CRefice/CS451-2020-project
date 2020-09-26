#include "links.hpp"

static std::uint16_t local_port(Parser& parser) {
  for (auto& host : parser.hosts()) {
    if (host.id == parser.id()) {
      return host.port;
    }
  }
  throw std::runtime_error("no host with the given id found in the hosts file");
}

MessageLink::MessageLink(Parser& parser)
    : process_id(parser.id()), socket(local_port(parser)) {
  host_addrs.resize(parser.hosts().size());
  for (auto& host : parser.hosts()) {
    address_of(host.id) = udp::socket_address(host.ip, host.port);
  }
}

void MessageLink::send_to(unsigned long receiver_id) {
  auto msg = Message{process_id, next_sequence_num++};
  socket.send(address_of(receiver_id), msg);
}

MessageLink::Message MessageLink::recv() {
  Message msg{};
  socket.recv(reinterpret_cast<char*>(&msg), sizeof(msg));
  return msg;
}
