#include "fair-loss-link.hpp"

namespace msg {
FairLossLink::FairLossLink(Parser& parser, udp::Socket& socket)
    : id(parser.index()), socket(socket) {
  addrs.reserve(parser.hosts().size());
  for (const auto& host : parser.hosts()) {
    addrs.push_back(udp::socket_address(host.ip, host.port));
  }
}

void FairLossLink::connect(Observer* o) { obs = o; }

void FairLossLink::send(ProcessId receiver, Message& msg) {
  msg.sender = id;

  const auto size = msg.size_bytes();
  // We're doing something really unsafe and dangerous here:
  // we rely on the ordering of the members of StaticVec to reinterpret
  // the message struct as a char vector of unknown (but bounded) size. Freaky!
  // Note that this is okay since the C standard specifies that the first member
  // of a struct may not be padded.
  const char* buf = reinterpret_cast<const char*>(&msg);
  socket.send(addrs[receiver], buf, size);
}

void FairLossLink::receive(const Message& msg) { obs->deliver(msg); }
} // namespace msg
