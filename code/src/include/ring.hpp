#pragma once

#include <array>
#include <atomic>
#include <optional>

template <typename T, int N = 1024>
class RingBuffer {
public:
  bool try_push(T&& value) {
    const auto idx = head.load(std::memory_order_acquire);
    const auto new_idx = increment(idx);
    if (new_idx == tail.load(std::memory_order_acquire)) {
      return false;
    }
    arr[idx] = value;
    head.store(new_idx, std::memory_order_release);
  }

  std::optional<T> try_pop() {
    auto idx = tail.load(std::memory_order_acquire);
    if (idx == head.load(std::memory_order_acquire)) {
      return std::nullopt;
    }
    tail.store(increment(idx), std::memory_order_release);
    return std::make_optional(std::move(arr[idx]));
  }

  T* front() {
    auto idx = tail.load(std::memory_order_acquire);
    if (idx == head.load(std::memory_order_acquire)) {
      return nullptr;
    }
    return &arr[idx];
  }

private:
  int increment(int index) { return index + 1 % N; }

  std::atomic<int> head, tail;
  std::array<T, N> arr;
};
