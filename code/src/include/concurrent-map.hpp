#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

template <typename T>
class ConcurrentQueue {
public:
  void push(T val) {
    {
      std::lock_guard lk(mutex);
      inner.push_back(std::move(val));
    }
    cv.notify_one();
  }

  template <class Clock, class Duration>
  std::optional<T>
  try_pop_until(std::chrono::time_point<Clock, Duration> time_point) {
    std::unique_lock lk(mutex);
    if (cv.wait_until(lk, time_point, [this] { return !inner.empty(); })) {
      // Item is present in the queue
      const auto item = std::move(inner.front());
      inner.pop_front();
      return item;
    } else {
      // Timeout
      return std::nullopt;
    }
  }

  T pop() {
    std::unique_lock lk(mutex);
    cv.wait(lk, [this] { return !inner.empty(); });
    const auto item = std::move(inner.front());
    inner.pop_front();
    return item;
  }

private:
  std::mutex mutex;
  std::condition_variable cv;
  std::deque<T> inner;
};
