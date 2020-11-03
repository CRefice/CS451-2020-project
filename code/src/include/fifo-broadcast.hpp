#pragma once

#include "monotonic-queue.hpp"
#include "uniform-reliable-broadcast.hpp"

namespace msg {
class FifoBroadcast : public Observer {
public:
  FifoBroadcast(Parser& parser, FairLossLink& link, Observer& observer);

  void send(BroadcastSeqNum sequence_num);

private:
  void deliver(const Message& msg) override;

  std::vector<MonotonicQueue<Message, BroadcastSeqNum>> ordered;
  UniformReliableBroadcast bc;
  Observer& observer;
};
} // namespace msg
