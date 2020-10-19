#include <chrono>
#include <unordered_set>

#include "link.hpp"

namespace msg {
// returns true if the given message was sent as a "Syn",
// false if it was sent as an "Ack"
static bool is_syn(const Message& msg) {
  // Since message sequence numbers can only go up to INT_MAX, we use the
  // most significant bit to keep track of whether it's syn/ack.
  auto msb = msg.broadcast_id.sequence_num & UINT_MSB;
  return msb == 0;
}

static Message set_ack(Message msg) {
  msg.broadcast_id.sequence_num |= UINT_MSB;
  return msg;
}

static Message unset_ack(Message msg) {
  msg.broadcast_id.sequence_num &= UINT_MSB;
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

FairLossLink::FairLossLink(Parser& parser)
    : process_id(static_cast<unsigned int>(parser.id())),
      socket(local_port(parser)) {
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

void FairLossLink::send(unsigned int receiver, Message msg) {
  msg.link_id.sender = process_id;
  socket.send(addrs[receiver - 1], msg);
}

struct Entry {
  Entry(Message msg)
      : msg(msg), retry_time(std::chrono::steady_clock::now() + timeout) {}
  Message msg;
  std::chrono::steady_clock::duration timeout = std::chrono::milliseconds(100);
  std::chrono::steady_clock::time_point retry_time;

  void reset() {
    timeout += std::chrono::milliseconds(100);
    retry_time = std::chrono::steady_clock::now() + timeout;
  }

  friend bool operator<(const Entry& a, const Entry& b) {
    return a.retry_time < b.retry_time;
  };
};

void PerfectLink::Peer::continuous_resync(ConcurrentQueue<Message>& incoming,
                                          unsigned int receiver,
                                          FairLossLink& link) {
  std::deque<Entry> to_send;
  std::unordered_set<unsigned int> acknowledged;
  to_send.emplace_back(incoming.pop());
  while (true) {
    auto next = to_send.front();
    std::optional<Message> maybe_item = std::nullopt;
    if (next.retry_time > std::chrono::steady_clock::now()) {
      maybe_item = incoming.try_pop_until(next.retry_time);
    }
    if (!maybe_item) {
      to_send.pop_front();
      link.send(receiver, next.msg);
      next.reset();

      to_send.push_back(next);
      continue;
    }
    auto msg = *maybe_item;
    // Invalid sender used to signal termination
    if (msg.link_id.sender > 128) {
      return;
    }
    if (is_syn(msg)) {
      to_send.emplace_back(msg);
    } else {
      acknowledged.insert(msg.link_id.sequence_num);
    }
    if (acknowledged.find(next.msg.link_id.sequence_num) !=
        acknowledged.end()) {
      to_send.pop_front();
      acknowledged.erase(next.msg.link_id.sequence_num);
    }
  }
} // namespace msg

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
  msg.link_id.sequence_num = peer.next_seq_num++;
  msg.link_id.sender = process_id;
  link.send(receiver, msg);
  peer.incoming.push(msg);
}

void PerfectLink::deliver(const Message& msg) {
  auto& peer = peers[msg.link_id.sender - 1];
  std::cout << msg << '\n';
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
