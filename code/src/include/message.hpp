#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <queue>
#include <unordered_map>

#include "static-vec.hpp"

namespace msg {
inline constexpr std::uint8_t MAX_PROCESSES = 128;

using LinkSeqNum = std::uint64_t;
using BroadcastSeqNum = std::uint32_t;
using ProcessId = std::uint8_t;
using VectorClock = StaticVec<BroadcastSeqNum, MAX_PROCESSES>;

struct Message {
  LinkSeqNum link_seq_num;
  BroadcastSeqNum bcast_seq_num;
  ProcessId originator, sender;
  VectorClock vector_clock;

  [[nodiscard]] std::size_t size_bytes() const noexcept {
    return offsetof(Message, vector_clock) +
           sizeof(BroadcastSeqNum) * vector_clock.size();
  }
};

struct BroadcastMessageHash {
  std::size_t operator()(const Message& m) const noexcept {
    std::size_t h1 = std::hash<BroadcastSeqNum>{}(m.bcast_seq_num);
    std::size_t h2 = std::hash<ProcessId>{}(m.originator);
    return h1 ^ (h2 << 1);
  }
};

struct BroadcastMessageCompare {
  bool operator()(const Message& a, const Message& b) const noexcept {
    return a.bcast_seq_num == b.bcast_seq_num && a.originator == b.originator;
  }
};

struct BroadcastMessageOrder {
  bool operator()(const Message& a, const Message& b) const noexcept {
    return a.bcast_seq_num > b.bcast_seq_num;
  }
};

template <typename T>
using BroadcastMessageHashMap =
    std::unordered_map<Message, T, BroadcastMessageHash,
                       BroadcastMessageCompare>;

using BroadcastMessageHeap =
    std::priority_queue<Message, std::vector<Message>, BroadcastMessageOrder>;

class Observer {
public:
  virtual void deliver(const Message& msg) = 0;
};
} // namespace msg
