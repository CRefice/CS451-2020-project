#pragma once

#include <deque>
#include <unordered_set>

#include "concurrent-queue.hpp"
#include "fair-loss-link.hpp"
#include "rolling-set.hpp"
#include "task.hpp"

namespace msg {
class PerfectLink : public Observer {
public:
  PerfectLink(Parser& parser, FairLossLink& link, Observer& observer);

  void send(ProcessId receiver, Message& msg);
  void deliver(const Message& msg) override;

private:
  struct Peer {
    Peer(ProcessId id, FairLossLink& link);

    LinkSeqNum next_seq_num = 0;
    RollingBitset delivered;
    // Data shared with worker thread
    ConcurrentQueue<Message> incoming;
    Task sender;
  };

  ProcessId id;
  // Condition variables can't be moved around in memory,
  // so we need a deque to hold them.
  std::deque<Peer> peers;
  FairLossLink& link;
  Observer& observer;
};
} // namespace msg
