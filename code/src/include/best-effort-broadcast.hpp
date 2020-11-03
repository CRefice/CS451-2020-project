#pragma once

#include "perfect-link.hpp"

namespace msg {
class BestEffortBroadcast : public Observer {
public:
  BestEffortBroadcast(Parser& parser, FairLossLink& link, Observer& observer);

  void send(Message msg);

private:
  void deliver(const Message& msg) override;

  std::uint8_t num_processes;
  PerfectLink link;
  Observer& observer;
};
} // namespace msg
