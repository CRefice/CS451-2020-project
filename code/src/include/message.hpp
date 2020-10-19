#pragma once

#include <functional>
#include <ostream>

inline unsigned int UINT_MSB = 1u << (sizeof(unsigned int) * 8 - 1);
inline unsigned int SYN_MASK = ~UINT_MSB;

namespace msg {
struct Identifier {
  unsigned int sender, sequence_num;

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
    std::size_t h1 = std::hash<unsigned int>{}(m.sender);
    std::size_t h2 = std::hash<unsigned int>{}(m.sequence_num);
    return h1 ^ (h2 << 1); // or use boost::hash_combine
  }
};
} // namespace std

namespace msg {
struct Message {
  Identifier link_id, broadcast_id;

  friend std::ostream& operator<<(std::ostream& os, const Message& m) {
    auto flags = os.flags();
    os << "from " << m.link_id.sender << ' ';
    os.fill('0');
    os.width(4);
    os << m.link_id.sequence_num;
    if (m.broadcast_id.sequence_num & UINT_MSB) {
      os << "(ack)";
    }
    os.flags(flags);
    return os;
  }
};

class Observer {
public:
  virtual void deliver(const Message& msg) = 0;
};
} // namespace msg
