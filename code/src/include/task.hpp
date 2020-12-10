#pragma once

#include <atomic>
#include <iostream>
#include <thread>
#include <type_traits>

// A task is a cancelable thread that cancels and joins on destruction.
class Task {
public:
  using CancelToken = std::atomic<bool>;

  Task() = default;
  template <typename Fn,
            typename = std::enable_if_t<std::is_invocable_v<Fn, CancelToken&>>>
  Task(Fn&& fn)
      : handle([this, fn = std::forward<Fn>(fn)]() mutable { fn(cancelled); }) {
  }

  ~Task() {
    cancel();
    join();
  }

  void cancel() noexcept {
    // We do not care about this operation's order related to other
    // load/stores, as long as it eventually gets to the thread.
    cancelled.store(true, std::memory_order_relaxed);
  }

  void join() { handle.join(); }

private:
  CancelToken cancelled = false;
  std::thread handle;
};
