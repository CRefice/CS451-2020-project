#pragma once

#include <bitset>
#include <queue>

#include "best-effort-broadcast.hpp"

namespace msg {
class UniformReliableBroadcast : public Observer {
public:
  UniformReliableBroadcast(Parser& parser, FairLossLink& link,
                           Observer& observer);

  void send(BroadcastSeqNum seq_num);

private:
  bool can_deliver(const Message& msg);
  void try_deliver(const Message& msg);
  void deliver(const Message& msg) override;

  ProcessId id;
  std::uint8_t num_processes;
  BroadcastMessageHashMap<std::bitset<MAX_PROCESSES>> ack;
  std::vector<RollingBitset> pending, delivered;
  BestEffortBroadcast bc;
  Observer& observer;
};
} // namespace msg
