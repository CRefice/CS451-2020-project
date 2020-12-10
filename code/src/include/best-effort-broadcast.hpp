#pragma once

#include "perfect-link.hpp"

namespace msg {
class BestEffortBroadcast : public Observer {
public:
  BestEffortBroadcast(Parser& parser, FairLossLink& link, Observer& observer);

  void send(Message& msg);

private:
  void deliver(const Message& msg) override;

  ProcessId id;
  std::vector<ProcessId> connections;

  PerfectLink link;
  Observer& observer;
};
} // namespace msg
