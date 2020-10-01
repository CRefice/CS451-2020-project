#pragma once

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "message.hpp"
#include "parser.hpp"
#include "udp.hpp"
#include "window.hpp"

namespace msg {
class PerfectFifoLink : public Observer {
public:
  PerfectFifoLink(unsigned long process_id, const Parser::Host& host,
                  udp::Socket& socket, Observer& observer)
      : process_id(process_id),
        receiver_addr(udp::socket_address(host.ip, host.port)), socket(socket),
        observer(observer) {}

  void send(int broadcast_seq_num);
  void deliver(const Message& msg) override;

  void resynchronize();

private:
  void do_send(const Message& msg);

  unsigned long process_id;
  int next_seq_num = 0;
  sockaddr_in receiver_addr;
  std::unordered_map<int, Message> sent_msgs;
  WindowBuffer<msg::Message> delivered;

  udp::Socket& socket;
  Observer& observer;
};
} // namespace msg
