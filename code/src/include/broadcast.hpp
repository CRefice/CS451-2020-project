#pragma once

#include <bitset>
#include <queue>

#include "link.hpp"
#include "window.hpp"

namespace msg {
class BestEffortBroadcast : public Observer {
public:
  BestEffortBroadcast(Parser& parser, FairLossLink& link, Observer& observer);

  void send(Message msg);

private:
  void deliver(const Message& msg) override;

  unsigned int process_id;
  std::size_t num_processes;
  PerfectLink link;
  Observer& observer;
};

class UniformReliableBroadcast : public Observer {
public:
  UniformReliableBroadcast(Parser& parser, FairLossLink& link,
                           Observer& observer);

  void send(unsigned int sequence_num);

private:
  bool can_deliver(const Message& msg);
  void try_deliver(const Message& msg);
  void deliver(const Message& msg) override;

  unsigned int process_id;
  std::size_t num_processes;
  std::unordered_map<Identifier, std::bitset<128>> ack;
  std::unordered_set<Identifier> pending, delivered;
  BestEffortBroadcast bc;
  Observer& observer;
};

class FifoBroadcast : public Observer {
public:
  FifoBroadcast(Parser& parser, FairLossLink& link, Observer& observer);

  void send(unsigned int sequence_num);

private:
  void deliver(const Message& msg) override;

  std::vector<OrderedBuffer<Message>> ordered;
  UniformReliableBroadcast bc;
  Observer& observer;
};
} // namespace msg
