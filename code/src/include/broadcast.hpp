#pragma once

#include "link.hpp"
#include <vector>

namespace msg {
class Broadcast : public Observer {
public:
  Broadcast(Parser& parser, udp::Socket& socket, Observer& observer);

  void send(int broadcast_num);

  void receive(const Message& msg);
  void resynchronize();

private:
  void deliver(const Message& msg) override;

  std::vector<PerfectFifoLink> links;
  Observer& observer;
};
} // namespace msg
