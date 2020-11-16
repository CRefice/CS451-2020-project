#pragma once

#include <chrono>

class ExponentialBackoff {
public:
  using Clock = std::chrono::steady_clock;

  ExponentialBackoff(
      Clock::duration initial_timeout = std::chrono::milliseconds(100))
      : timeout(initial_timeout), retry_at(Clock::now() + timeout) {}

  void retry() {
    timeout *= 2;
    retry_at = Clock::now() + timeout;
  }

  Clock::time_point next_timeout() const noexcept { return retry_at; }

private:
  Clock::duration timeout;
  Clock::time_point retry_at;
};
