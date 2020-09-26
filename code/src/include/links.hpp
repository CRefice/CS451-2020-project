#pragma once

#include "parser.hpp"
#include "udp.hpp"

class MessageLink {
public:
  struct Message {
    unsigned long sender;
    int sequence_num;
  };

  MessageLink(Parser& parser);

  void send_to(unsigned long receiver_id);
  Message recv();

private:
  sockaddr_in& address_of(unsigned long receiver_id) noexcept {
    return host_addrs[receiver_id - 1];
  }

  int next_sequence_num = 0;
  unsigned long process_id;
  udp::Socket socket;
  std::vector<sockaddr_in> host_addrs;
};
