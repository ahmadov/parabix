#include "stream/bit_stream.h"
#include <cassert>

using BitStream = stream::BitStream;

BitStream BitStream::operator&(const BitStream& other) const {
  BitStream result(*this);
  return result &= other;
}

BitStream BitStream::operator|(const BitStream& other) const {
  BitStream result(*this);
  return result |= other;
}

BitStream BitStream::operator^(const BitStream& other) const {
  BitStream result(*this);
  return result ^= other;
}

BitStream BitStream::operator+(const BitStream& other) const {
  BitStream result(*this);
  return result += other;
}

BitStream BitStream::operator>>(const size_t offset) const {
  BitStream result(*this);
  return result >>= offset;
}

BitStream BitStream::operator~() const {
  BitStream result(*this);
  for (auto i = 0; i < blocks.size(); ++i) {
    result.blocks[i] = ~blocks[i];
  }
  return result;
}

BitStream& BitStream::operator&=(const BitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] &= other.blocks[i];
  }

  return *this;
}

BitStream& BitStream::operator|=(const BitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] |= other.blocks[i];
  }

  return *this;
}

BitStream& BitStream::operator^=(const BitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] ^= other.blocks[i];
  }

  return *this;
}

BitStream& BitStream::operator>>=(const size_t offset) {
  bool carry = false;
  for (auto& block : blocks) {
    bool next_carry = (block >> bits_per_block) & 1;
    block <<= offset;
    block |= carry;
    carry = next_carry;
  }
  return *this;
}

BitStream& BitStream::operator+=(const BitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  bool carry = 0;
  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] += other.blocks[i] + carry;
    carry = (blocks[i] >> bits_per_block) & 1;
    blocks[i] &= ~(1ULL << bits_per_block);
  }

  return *this;
}

void BitStream::set(size_t pos, bool value) {
  if (value) {
    set(pos);
  } else {
    // TODO(me): clear
  }
}

void BitStream::set(size_t pos) {
  auto [block_index, pos_in_block] = find_block(pos);
  blocks[block_index] |= 1ULL << pos_in_block;
}

void BitStream::clear(size_t pos) {
  auto [block_index, pos_in_block] = find_block(pos);
  blocks[block_index] &= ~(1ULL << pos_in_block);
}

bool BitStream::is_set(size_t pos) {
  auto [block_index, pos_in_block] = find_block(pos);
  return (blocks[block_index] >> pos_in_block) & 1;
}

std::pair<size_t, size_t> BitStream::find_block(size_t pos) {
  auto part = pos / bits_per_block;
  auto pos_in_block = pos - (bits_per_block * part);
  return {part, pos_in_block};
}
