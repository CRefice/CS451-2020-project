#pragma once

#include <functional>

namespace msg {
struct Identifier {
  unsigned long sender;
  int sequence_num;

  friend bool operator==(const Identifier& a, const Identifier& b) {
    return a.sender == b.sender && a.sequence_num == b.sequence_num;
  }
};
} // namespace msg

// from https://en.cppreference.com/w/cpp/utility/hash
// custom specialization of std::hash can be injected in namespace std
namespace std {
template <>
struct hash<msg::Identifier> {
  std::size_t operator()(const msg::Identifier& m) const noexcept {
    std::size_t h1 = std::hash<unsigned long>{}(m.sender);
    std::size_t h2 = std::hash<int>{}(m.sequence_num);
    return h1 ^ (h2 << 1); // or use boost::hash_combine
  }
};
} // namespace std

namespace msg {
enum Discriminant : char {
  Syn = 0,
  Ack = 1,
};

struct Message {
  Identifier id;
  int broadcast_seq_num;
  Discriminant discr;
};

class Observer {
public:
  virtual void deliver(const Message& msg) = 0;
};
} // namespace msg
