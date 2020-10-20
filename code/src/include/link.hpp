#pragma once

#include <deque>
#include <unordered_set>
#include <vector>

#include "concurrent-queue.hpp"
#include "message.hpp"
#include "parser.hpp"
#include "task.hpp"
#include "udp.hpp"
#include "window.hpp"

namespace msg {
class FairLossLink {
public:
  FairLossLink(Parser& parser, udp::Socket& socket);

  void connect(Observer* obs);
  void deliver(const Message& msg);

  void send(unsigned int receiver, Message msg);

private:
  unsigned int process_id;
  std::vector<sockaddr_in> addrs;
  udp::Socket& socket;
  Observer* obs = nullptr;
};

class PerfectLink : public Observer {
public:
  PerfectLink(Parser& parser, FairLossLink& link, Observer& observer);

  void send(unsigned int receiver, Message msg);
  void deliver(const Message& msg) override;

private:
  struct Peer {
    Peer(unsigned int process_id, FairLossLink& link);

    unsigned int next_seq_num = 0;
    WindowBuffer<unsigned int> delivered;
    // Data shared with worker thread
    ConcurrentQueue<Message> incoming;
    Task sender;
  };

  unsigned int process_id;
  // Condition variables can't be moved around in memory,
  // so we need a deque to hold them.
  std::deque<Peer> peers;
  FairLossLink& link;
  Observer& observer;
};
} // namespace msg
