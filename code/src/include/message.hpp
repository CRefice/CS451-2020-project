#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace msg {
using LinkSeqNum = std::uint64_t;
using BroadcastSeqNum = std::uint32_t;
using ProcessId = std::uint8_t;

inline constexpr std::uint8_t MAX_PROCESSES = 128;

class Message {
public:
  LinkSeqNum link_seq_num;
  BroadcastSeqNum bcast_seq_num;
  ProcessId originator, sender;
};

struct BroacastMessageHash {
  std::size_t operator()(const Message& m) const noexcept {
    std::size_t h1 = std::hash<BroadcastSeqNum>{}(m.bcast_seq_num);
    std::size_t h2 = std::hash<ProcessId>{}(m.originator);
    return h1 ^ (h2 << 1);
  }
};

struct BroacastMessageCompare {
  bool operator()(const Message& a, const Message& b) const noexcept {
    return a.bcast_seq_num == b.bcast_seq_num && a.originator == b.originator;
  }
};

template <typename T>
using BroadcastMessageHashMap =
    std::unordered_map<Message, T, BroacastMessageHash, BroacastMessageCompare>;

using BroadcastMessageHashSet =
    std::unordered_set<Message, BroacastMessageHash, BroacastMessageCompare>;

class Observer {
public:
  virtual void deliver(const Message& msg) = 0;
};
} // namespace msg
