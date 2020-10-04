#pragma once

#include "link.hpp"
#include "window.hpp"

namespace msg {
class Broadcast : public Observer {
public:
  Broadcast(Parser& parser, udp::Socket& socket, Observer& observer);

  void send(const Message& msg);

  void receive(const Message& msg);
  void resynchronize();

private:
  void deliver(const Message& msg) override;

  std::vector<PerfectFifoLink> links;
  Observer& observer;
};

class UniformReliableBroadcast : public Observer {
public:
  UniformReliableBroadcast(Parser& parser, udp::Socket& socket,
                           Observer& observer)
      : process_id(parser.id()), num_processes(parser.hosts().size()),
        bc(parser, socket, *this), observer(observer) {}

  void send(int broadcast_num);
  void resynchronize() { bc.resynchronize(); }
  void receive(const Message& msg) { bc.receive(msg); }

private:
  bool can_deliver(const Message& msg);

  void deliver(const Message& msg) override;

  unsigned long process_id, num_processes;
  std::unordered_map<Identifier, std::unordered_set<unsigned long>> ack;
  std::unordered_set<Identifier> pending;
  std::unordered_set<Identifier> delivered;
  Broadcast bc;
  Observer& observer;
};
} // namespace msg
