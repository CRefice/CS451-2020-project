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

FairLossLink::FairLossLink(Parser& parser, udp::Socket& socket)
    : process_id(static_cast<unsigned int>(parser.id())), socket(socket) {
  addrs.reserve(parser.hosts().size());
  for (const auto& host : parser.hosts()) {
    addrs.push_back(udp::socket_address(host.ip, host.port));
  }
}

void FairLossLink::connect(Observer* o) { obs = o; }

void FairLossLink::deliver(const Message& msg) { obs->deliver(msg); }

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

PerfectLink::Peer::Peer(unsigned int process_id, FairLossLink& link)
    : delivered(), incoming(),
      sender([this, receiver = process_id, &link](Task::CancelToken& cancel) {
        auto to_send = std::deque<Entry>();
        auto acknowledged = std::unordered_set<unsigned int>();
        to_send.emplace_back(incoming.pop());
        while (!cancel) {
          std::optional<Message> maybe_item = std::nullopt;
          if (to_send.empty()) {
            maybe_item = incoming.pop();
          } else {
            const auto& next = to_send.front();
            if (next.retry_time > std::chrono::steady_clock::now()) {
              maybe_item = incoming.try_pop_until(next.retry_time);
            }
          }
          if (!maybe_item) {
            auto next = to_send.front();
            to_send.pop_front();
            if (acknowledged.find(next.msg.link_id.sequence_num) !=
                acknowledged.end()) {
              acknowledged.erase(next.msg.link_id.sequence_num);
            } else {
              link.send(receiver, next.msg);
              next.reset();
              to_send.push_back(next);
            }
            continue;
          }
          auto msg = *maybe_item;
          if (is_syn(msg)) {
            to_send.emplace_back(msg);
          } else {
            acknowledged.insert(msg.link_id.sequence_num);
          }
        }
      }) {}

PerfectLink::PerfectLink(Parser& parser, FairLossLink& link, Observer& observer)
    : process_id(static_cast<unsigned int>(parser.id())), link(link),
      observer(observer) {
  link.connect(this);
  for (const auto& host : parser.hosts()) {
    peers.emplace_back(static_cast<unsigned int>(host.id), link);
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
  if (!is_syn(msg)) {
    peer.incoming.push(msg);
    return;
  }
  const auto seq_num = msg.link_id.sequence_num;
  if (!peer.delivered.has_seen(seq_num)) {
    peer.delivered.add(seq_num);
    observer.deliver(msg);
  }
  link.send(msg.link_id.sender, set_ack(msg));
}
} // namespace msg
