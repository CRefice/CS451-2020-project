#pragma once

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "message.hpp"
#include "parser.hpp"
#include "udp.hpp"

namespace msg {
class Link : public Observer {
public:
  Link(Parser& parser, udp::Socket& socket, Observer& observer);

  void send(unsigned long receiver, int broadcast_seq_num);
  void deliver(const Message& msg) override;

  void resynchronize();

private:
  void do_send(unsigned long receiver, const Message& msg);

  sockaddr_in& address_of(unsigned long receiver_id) noexcept {
    return host_addrs[receiver_id - 1];
  }

  unsigned long process_id;
  int next_seq_num = 0;
  std::vector<sockaddr_in> host_addrs;
  std::unordered_map<int, std::pair<Message, unsigned long>> sent_msgs;
  udp::Socket& socket;
  Observer& observer;
};

class PerfectLink : public Link {
public:
  PerfectLink(Parser& parser, udp::Socket& socket, Observer& observer)
      : Link(parser, socket, *this), observer(observer) {}

  void deliver(const Message& msg) override;

private:
  std::unordered_set<Identifier> delivered_msgs;
  Observer& observer;
};
} // namespace msg
