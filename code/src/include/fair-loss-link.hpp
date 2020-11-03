#pragma once

#include <vector>

#include "message.hpp"
#include "parser.hpp"
#include "udp.hpp"

namespace msg {
class FairLossLink {
public:
  FairLossLink(Parser& parser, udp::Socket& socket);

  void send(ProcessId receiver, Message msg);
  void receive(const Message& msg);

  void connect(Observer* obs);

private:
  ProcessId id;
  std::vector<sockaddr_in> addrs;
  udp::Socket& socket;
  Observer* obs = nullptr;
};
} // namespace msg
