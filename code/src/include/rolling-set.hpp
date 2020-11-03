#pragma once

#include <bitset>
#include <deque>

class RollingBitset {
public:
  void insert(std::size_t elem);

  bool contains(std::size_t elem) const noexcept;

private:
  static constexpr std::size_t CHUNK_SIZE = 512;
  struct Chunk {
    std::bitset<CHUNK_SIZE> bits;
    std::uint16_t count = 0;

    void set(std::size_t idx) noexcept {
      if (!bits.test(idx)) {
        count++;
      }
      bits.set(idx);
    }

    bool test(std::size_t idx) const noexcept { return bits.test(idx); }
  };

  bool has_passed(std::size_t elem) const noexcept;
  std::size_t chunk_index(std::size_t elem) const noexcept;
  void rotate_chunks();

  std::size_t offset = 0;
  std::deque<Chunk> chunks;
};
