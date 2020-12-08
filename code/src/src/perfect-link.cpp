#include <chrono>
#include <queue>
#include <unordered_set>

#include "exponential-backoff.hpp"

#include "perfect-link.hpp"

namespace msg {
LinkSeqNum SEQ_NUM_MSB = 1ul << (sizeof(LinkSeqNum) * 8 - 1);
LinkSeqNum SEQ_NUM_MASK = ~SEQ_NUM_MSB;

// returns true if the given message was sent as a "Syn",
// false if it was sent as an "Ack"
static bool is_syn(const Message& msg) {
  auto msb = msg.link_seq_num & SEQ_NUM_MSB;
  return msb == 0;
}

static Message set_ack(Message msg) {
  msg.link_seq_num |= SEQ_NUM_MSB;
  return msg;
}

static LinkSeqNum seq_num(const Message& msg) {
  return msg.link_seq_num & SEQ_NUM_MASK;
}

struct Entry {
  Message msg;
  ExponentialBackoff backoff;

  Entry(Message msg) : msg(msg) {}

  friend bool operator>(const Entry& a, const Entry& b) {
    return a.backoff.next_timeout() > b.backoff.next_timeout();
  };
};

using EntryQueue =
    std::priority_queue<Entry, std::vector<Entry>, std::greater<>>;

static std::optional<Message> try_pop(ConcurrentQueue<Message>& queue,
                                      const EntryQueue& scheduled) {
  if (scheduled.empty()) {
    // Sleep for max 500ms, then bail
    // This is in order to avoid sleeping forever (and thus not terminating)
    // if there are no more items in the queue at the end of execution.
    const auto timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    return queue.try_pop_until(timeout);
  }
  const auto retry_time = scheduled.top().backoff.next_timeout();
  if (retry_time > ExponentialBackoff::Clock::now()) {
    return queue.try_pop_until(retry_time);
  }
  return std::nullopt;
}

static void handle_timeout(EntryQueue& scheduled,
                           std::unordered_set<LinkSeqNum>& acknowledged,
                           ProcessId receiver, FairLossLink& link) {
  if (scheduled.empty()) {
    return;
  }
  auto next = scheduled.top();
  scheduled.pop();
  const auto num = seq_num(next.msg);
  if (acknowledged.find(num) != acknowledged.end()) {
    acknowledged.erase(num);
  } else {
    link.send(receiver, next.msg);
    next.backoff.retry();
    scheduled.push(next);
  }
}

PerfectLink::Peer::Peer(ProcessId id, FairLossLink& link)
    : sender([this, receiver = id, &link](Task::CancelToken& cancel) {
        auto scheduled = EntryQueue();
        auto acknowledged = std::unordered_set<LinkSeqNum>();
        while (!cancel.load(std::memory_order_relaxed)) {
          std::optional<Message> maybe_item = try_pop(incoming, scheduled);
          if (!maybe_item) {
            handle_timeout(scheduled, acknowledged, receiver, link);
            continue;
          }
          auto msg = *maybe_item;
          if (is_syn(msg)) {
            scheduled.push(msg);
          } else {
            acknowledged.insert(seq_num(msg));
          }
        }
      }) {}

PerfectLink::PerfectLink(Parser& parser, FairLossLink& link, Observer& observer)
    : id(parser.index()), link(link), observer(observer) {
  link.connect(this);
  for (const auto& host : parser.hosts()) {
    peers.emplace_back(host.index(), link);
  }
}

void PerfectLink::send(ProcessId receiver, Message& msg) {
  auto& peer = peers[receiver];
  msg.link_seq_num = peer.next_seq_num++;
  msg.sender = id;
  peer.incoming.push(msg);
  link.send(receiver, msg);
}

void PerfectLink::deliver(const Message& msg) {
  auto& peer = peers[msg.sender];
  if (!is_syn(msg)) {
    peer.incoming.push(msg);
    return;
  }
  const auto num = seq_num(msg);
  if (!peer.delivered.contains(num)) {
    peer.delivered.insert(num);
    observer.deliver(msg);
  }
  auto ack = set_ack(msg);
  link.send(msg.sender, ack);
}
} // namespace msg
