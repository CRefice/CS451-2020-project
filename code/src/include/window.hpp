#pragma once

#include <array>
#include <cassert>
#include <queue>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "message.hpp"

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
class WindowBuffer {
public:
  void add(T item) noexcept {
    inserted.insert(item);
    typename std::unordered_set<T>::iterator it;
    while ((it = inserted.find(expected_next)) != inserted.end()) {
      inserted.erase(it);
      expected_next++;
    }
  }

  bool has_seen(T item) const noexcept {
    return item < offset || inserted.find(item) != inserted.end();
  }

private:
  T offset = 0, expected_next = 0;
  std::unordered_set<T> inserted;
};

template <typename T>
class OrderedBuffer {
public:
  void add(T item, unsigned int seq_num) noexcept {
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
    unsigned int id;

    friend bool operator>(const Entry& a, const Entry& b) noexcept {
      return a.id > b.id;
    }
  };

  // Sort by smallest element on top
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> queue;
  unsigned int expected_next = 0;
};
