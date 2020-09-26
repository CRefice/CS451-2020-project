#pragma once

#include <unordered_map>

#include "parser.hpp"
#include "udp.hpp"

class MessageLink {
public:
  struct Message {
    unsigned long sender;
    int sequence_num;
  };

  MessageLink(Parser& parser);

  void send_to(unsigned long receiver_id, int sequence_num);
  std::optional<Message> try_recv();

  void send_heartbeats();

private:
  void send(unsigned long receiver_id, const Message& msg);

  sockaddr_in& address_of(unsigned long receiver_id) noexcept {
    return host_addrs[receiver_id - 1];
  }

  unsigned long process_id;
  udp::Socket socket;
  std::vector<sockaddr_in> host_addrs;
  std::unordered_map<int, unsigned long> sent_msgs;
};
