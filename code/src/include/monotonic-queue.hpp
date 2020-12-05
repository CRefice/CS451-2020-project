#pragma once

#include <queue>
#include <type_traits>
#include <utility>

#include "message.hpp"

template <typename T, typename Index,
          typename = std::enable_if_t<std::is_integral_v<Index>>>
class MonotonicQueue {
public:
  void add(T item, Index seq_num) {
    queue.push(Entry{std::move(item), seq_num});
  }

  bool has_next() const noexcept { return queue.top().id == expected_next; }

  T pop_next() {
    auto entry = std::move(queue.top());
    queue.pop();
    expected_next++;
    return entry.val;
  }

private:
  struct Entry {
    T val;
    Index id;

    friend bool operator>(const Entry& a, const Entry& b) noexcept {
      return a.id > b.id;
    }
  };

  // Sort by smallest element on top
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> queue;
  Index expected_next = 0;
};
