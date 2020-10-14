#include <unordered_map>

#include "link.hpp"

namespace msg {
static constexpr unsigned int UINT_MSB = 1u << (sizeof(unsigned int) * 8 - 1);
static constexpr unsigned int SYN_MASK = ~UINT_MSB;

// returns true if the given message was sent as a "Syn",
// false if it was sent as an "Ack"
static bool is_syn(const Message& msg) {
  // Since message sequence numbers can only go up to INT_MAX, we use the
  // most significant bit to keep track of whether it's syn/ack.
  auto msb = msg.link_id.sequence_num & UINT_MSB;
  return msb == 0;
}

static Message set_ack(Message msg) {
  msg.link_id.sequence_num |= UINT_MSB;
  return msg;
}

static std::uint16_t local_port(Parser& parser) {
  for (auto& host : parser.hosts()) {
    if (host.id == parser.id()) {
      return host.port;
    }
  }
  throw std::runtime_error("no host with the given id found in the hosts file");
}

FairLossLink::FairLossLink(Parser& parser) : socket(local_port(parser)) {
  addrs.reserve(parser.hosts().size());
  for (const auto& host : parser.hosts()) {
    addrs.push_back(udp::socket_address(host.ip, host.port));
  }
}

void FairLossLink::connect(Observer* o) { obs = o; }

void FairLossLink::try_deliver() {
  Message msg;
  if (obs && socket.try_recv(reinterpret_cast<char*>(&msg), sizeof(msg))) {
    obs->deliver(msg);
  }
}

void FairLossLink::deliver() {
  Message msg;
  socket.recv(reinterpret_cast<char*>(&msg), sizeof(msg));
  obs->deliver(msg);
}

void FairLossLink::send(unsigned int receiver, const Message& msg) {
  socket.send(addrs[receiver - 1], msg);
}

void PerfectLink::Peer::continuous_resync(ConcurrentQueue<Message>& incoming,
                                          unsigned int receiver,
                                          FairLossLink& link) {
  std::unordered_map<unsigned int, Message> sent;
  const auto wakeup_time = [] {
    return std::chrono::high_resolution_clock::now() +
           std::chrono::milliseconds(100);
  };
  auto wakeup_point = wakeup_time();
  while (true) {
    auto maybe_item = incoming.try_pop_until(wakeup_point);
    if (!maybe_item) {
      for (const auto& [_, msg] : sent) {
        link.send(receiver, msg);
      }
      wakeup_point = wakeup_time();
      continue;
    }
    auto msg = *maybe_item;
    // Invalid sender used to signal termination
    if (msg.link_id.sender > 128) {
      return;
    }
    if (is_syn(msg)) {
      sent.emplace(msg.link_id.sequence_num & SYN_MASK, msg);
    } else {
      sent.erase(msg.link_id.sequence_num & SYN_MASK);
    }
  }
}

PerfectLink::PerfectLink(Parser& parser, FairLossLink& link, Observer& observer)
    : process_id(static_cast<unsigned int>(parser.id())), link(link),
      observer(observer) {
  link.connect(this);
  for (const auto& host : parser.hosts()) {
    peers.emplace_back(static_cast<unsigned int>(host.id), link);
  }
}

PerfectLink::~PerfectLink() {
  // Terminate threads
  for (auto& peer : peers) {
    Message dummy{};
    dummy.link_id.sender = 129;
    peer.incoming.push(dummy);
  }
  for (auto& peer : peers) {
    peer.syn_worker.join();
  }
}

void PerfectLink::send(unsigned int receiver, Message msg) {
  auto& peer = peers[receiver - 1];
  msg.link_id.sender = process_id;
  msg.link_id.sequence_num = peer.next_seq_num++;
  link.send(receiver, msg);
  peer.incoming.push(msg);
}

void PerfectLink::deliver(const Message& msg) {
  auto& peer = peers[msg.link_id.sender - 1];
  if (!is_syn(msg)) {
    peer.incoming.push(msg);
    return;
  }
  const auto seq_num = msg.link_id.sequence_num;
  bool should_ack = true;
  if (!peer.delivered.has_seen(seq_num)) {
    should_ack = peer.delivered.add(seq_num);
    if (should_ack) {
      observer.deliver(msg);
    }
  }
  if (should_ack) {
    link.send(msg.link_id.sender, set_ack(msg));
  }
}
} // namespace msg
