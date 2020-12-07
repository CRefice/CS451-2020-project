#pragma once

#include <queue>

#include "config-parser.hpp"
#include "monotonic-queue.hpp"
#include "uniform-reliable-broadcast.hpp"

namespace msg {
class LocalizedCausalBroadcast : public Observer {
public:
  LocalizedCausalBroadcast(ConfigParser& config, Parser& parser,
                           FairLossLink& link, Observer& observer);

  void send(BroadcastSeqNum sequence_num);

private:
  bool can_deliver(const Message& msg);

  void deliver(const Message& msg) override;

  ProcessId id;
  std::vector<ConfigParser::Dependency> deps;
  std::vector<Message> pending;

  VectorClock current_clock;

  UniformReliableBroadcast bc;
  Observer& observer;
};
} // namespace msg
