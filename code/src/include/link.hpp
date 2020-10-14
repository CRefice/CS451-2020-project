#pragma once

#include <deque>
#include <thread>
#include <unordered_set>
#include <vector>

#include "concurrent-map.hpp"
#include "message.hpp"
#include "parser.hpp"
#include "udp.hpp"
#include "window.hpp"

namespace msg {
class FairLossLink {
public:
  FairLossLink(Parser& parser);

  void connect(Observer* obs);
  void try_deliver();
  void deliver();

  void send(unsigned int receiver, const Message& msg);

private:
  std::vector<sockaddr_in> addrs;
  udp::Socket socket;
  Observer* obs = nullptr;
};

class PerfectLink : public Observer {
public:
  PerfectLink(Parser& parser, FairLossLink& link, Observer& observer);
  ~PerfectLink();

  void send(unsigned int receiver, Message msg);
  void deliver(const Message& msg) override;

private:
  struct Peer {
    Peer(unsigned int process_id, FairLossLink& link)
        : delivered(), incoming(), syn_worker([this, process_id, &link] {
            continuous_resync(incoming, process_id, link);
          }) {}

    static void continuous_resync(ConcurrentQueue<Message>& incoming,
                                  unsigned int receiver, FairLossLink& link);

    unsigned int next_seq_num = 0;
    WindowBuffer<unsigned int> delivered;
    // Data shared with worker thread
    ConcurrentQueue<Message> incoming;
    std::thread syn_worker;
  };

  unsigned int process_id;
  // Condition variables can't be moved around in memory,
  // so we need a deque to hold them.
  std::deque<Peer> peers;
  FairLossLink& link;
  Observer& observer;
};
} // namespace msg
