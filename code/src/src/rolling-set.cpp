#include <iostream>

#include "rolling-set.hpp"

void RollingBitset::insert(std::size_t elem) {
  if (has_passed(elem)) {
    return;
  }
  const auto chunk_idx = chunk_index(elem);
  while (chunk_idx >= chunks.size()) {
    chunks.emplace_back();
  }
  const auto bit_idx = elem % CHUNK_SIZE;
  chunks[chunk_idx].set(bit_idx);
  if (chunk_idx == 0 && chunks[chunk_idx].count == CHUNK_SIZE) {
    rotate_chunks();
  }
}

bool RollingBitset::contains(std::size_t elem) const noexcept {
  if (has_passed(elem)) {
    return true;
  }
  const auto chunk_idx = chunk_index(elem);
  if (chunk_idx >= chunks.size()) {
    return false;
  }
  const auto bit_idx = elem % CHUNK_SIZE;
  return chunks[chunk_idx].test(bit_idx);
}

void RollingBitset::rotate_chunks() {
  chunks.pop_front();
  offset++;
}

std::size_t RollingBitset::chunk_index(std::size_t elem) const noexcept {
  return (elem / CHUNK_SIZE) - offset;
}

bool RollingBitset::has_passed(std::size_t elem) const noexcept {
  return (elem / CHUNK_SIZE) < offset;
}
