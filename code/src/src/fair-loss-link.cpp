#include "fair-loss-link.hpp"

namespace msg {
FairLossLink::FairLossLink(Parser& parser, udp::Socket& socket)
    : id(static_cast<ProcessId>(parser.id())), socket(socket) {
  addrs.reserve(parser.hosts().size());
  for (const auto& host : parser.hosts()) {
    addrs.push_back(udp::socket_address(host.ip, host.port));
  }
}

void FairLossLink::connect(Observer* o) { obs = o; }

void FairLossLink::send(ProcessId receiver, Message msg) {
  msg.sender = id;
  socket.send(addrs[receiver - 1], msg);
}

void FairLossLink::receive(const Message& msg) { obs->deliver(msg); }
} // namespace msg
