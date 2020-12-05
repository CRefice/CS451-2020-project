#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>

/// A stack-allocated vector that can hold at most `Cap` elements of type T.
template <typename T, std::size_t Cap>
class StaticVec {
public:
  StaticVec() noexcept = default;
  StaticVec(std::size_t size) noexcept : sz(size) { storage.fill(T{}); }

  StaticVec(const StaticVec& v) noexcept : sz(v.sz) {
    std::memcpy(storage.data(), v.storage.data(), sz * sizeof(T));
  }
  StaticVec(StaticVec&& v) noexcept : sz(v.sz) {
    std::memcpy(storage.data(), v.storage.data(), sz * sizeof(T));
  }
  ~StaticVec() noexcept = default;

  StaticVec& operator=(const StaticVec& v) noexcept {
    sz = v.sz;
    std::memcpy(storage.data(), v.storage.data(), sz * sizeof(T));
    return *this;
  }
  StaticVec& operator=(StaticVec&& v) noexcept {
    sz = v.sz;
    std::memcpy(storage.data(), v.storage.data(), sz * sizeof(T));
    return *this;
  }

  void push_back(T item) noexcept { storage[sz++] = item; }
  T pop_back() noexcept { return storage[sz--]; }

  // Really unsafe stuff (hence why the name `force`), used to tell a vector
  // its size after it has been deserialized from an array of bytes.
  void force_set_size(std::size_t size) noexcept { sz = size; }
  std::size_t size() const noexcept { return sz; }

  T& operator[](std::size_t idx) noexcept { return storage[idx]; }
  const T& operator[](std::size_t idx) const noexcept { return storage[idx]; }

  T* begin() noexcept { return storage.begin(); }
  const T* begin() const noexcept { return storage.begin(); }
  T* end() noexcept { return begin() + sz; }
  const T* end() const noexcept { return begin() + sz; }

private:
  std::array<T, Cap> storage;
  std::size_t sz = 0;
};
