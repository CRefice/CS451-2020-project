#pragma once

#include <array>
#include <cassert>
#include <queue>
#include <type_traits>
#include <utility>

#include "message.hpp"

template <typename T, std::size_t Cap = 10 * 1024,
          typename = std::enable_if_t<std::is_integral_v<T>>>
class WindowBuffer {
public:
  bool add(T item) noexcept {
    if (has_seen(item)) {
      // Message has already been added before, so no bother.
      return true;
    }
    const auto idx = index_of(item);
    if (idx >= Cap) {
      return false;
    }
    marks[idx] = true;
    if (++size == Cap) {
      std::cout << "clearing\n";
      offset += Cap;
      size = 0;
      marks.fill(false);
    }
    return true;
  }

  bool has_seen(T item) const noexcept {
    return item < offset || marks[index_of(item)];
  }

private:
  std::size_t index_of(T item) const noexcept { return item - offset; }

  T offset = 0, size = 0;
  std::array<bool, Cap> marks{};
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

    friend bool operator<(const Entry& a, const Entry& b) {
      return a.id < b.id;
    }
  };

  std::priority_queue<Entry> queue;
  unsigned int expected_next = 0;
};
