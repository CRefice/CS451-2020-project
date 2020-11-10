#pragma once

#include "perfect-link.hpp"

namespace msg {
class BestEffortBroadcast : public Observer {
public:
  BestEffortBroadcast(Parser& parser, FairLossLink& link, Observer& observer);

  void send(Message msg);

private:
  void deliver(const Message& msg) override;

  void create_n_connected_ring(ProcessId n);

  std::vector<ProcessId> connections;
  ProcessId id;
  PerfectLink link;
  Observer& observer;
};
} // namespace msg
