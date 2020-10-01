#pragma once

#include <array>
#include <cassert>

#include "message.hpp"

template <typename T>
class WindowBuffer {
public:
  static constexpr int CAPACITY = 2;

  template <typename Fn>
  bool add(T item, int seq_num, Fn&& callback) noexcept {
    const auto idx = index_of(seq_num);
    if (idx >= CAPACITY) {
      return false;
    }
    arr[idx] = item;
    marks[idx] = true;
    while (expected_next <= idx && marks[expected_next]) {
      callback(arr[expected_next++]);
    }
    if (expected_next == CAPACITY) {
      marks.fill(false);
      offset += CAPACITY;
      expected_next = 0;
    }
    return true;
  }

  bool has_seen(int seq_num) const noexcept {
    return seq_num < offset || marks[index_of(seq_num)];
  }

  int current_offset() const noexcept { return offset; }

private:
  int index_of(int seq_num) const noexcept { return seq_num - offset; }

  int offset = 0, expected_next = 0;
  std::array<T, CAPACITY> arr{};
  std::array<bool, CAPACITY> marks{};
};
