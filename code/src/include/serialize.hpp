#pragma once

#include <cstdint>
#include <cstring>

template <typename T, typename... Args>
inline std::size_t serialization_buffer_size(T*, std::size_t count,
                                             Args... args) {
  return sizeof(T) * count + serialization_buffer_size(args...);
}

template <typename T, typename... Args>
inline std::size_t serialization_buffer_size(T, Args... args) {
  return sizeof(T) + serialization_buffer_size(args...);
}

inline std::size_t serialization_buffer_size() { return 0; }

template <typename T, typename... Args>
inline void serialize(char* dst, T* src, std::size_t count, Args... args) {
  const std::size_t len = sizeof(T) * count;
  std::memcpy(dst, src, len);
  serialize(dst + len, args...);
}

template <typename T, typename... Args>
inline void serialize(char* dst, T src, Args... args) {
  const std::size_t len = sizeof(T);
  std::memcpy(dst, &src, len);
  serialize(dst + len, args...);
}

inline void serialization_buffer_size(char*) {}
