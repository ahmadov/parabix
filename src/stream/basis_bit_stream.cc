#include "stream/basis_bit_stream.h"
#include <cassert>

using BasisBitStream = stream::BasisBitStream;

BasisBitStream BasisBitStream::operator&(const BasisBitStream& other) const {
  BasisBitStream result(*this);
  return result &= other;
}

BasisBitStream BasisBitStream::operator|(const BasisBitStream& other) const {
  BasisBitStream result(*this);
  return result |= other;
}

BasisBitStream BasisBitStream::operator^(const BasisBitStream& other) const {
  BasisBitStream result(*this);
  return result ^= other;
}

BasisBitStream BasisBitStream::operator+(const BasisBitStream& other) const {
  BasisBitStream result(*this);
  return result += other;
}

BasisBitStream BasisBitStream::operator>>(const size_t offset) const {
  BasisBitStream result(*this);
  return result >>= offset;
}

BasisBitStream BasisBitStream::operator~() const {
  BasisBitStream result(*this);
  for (auto i = 0; i < blocks.size(); ++i) {
    result.blocks[i] = ~blocks[i];
  }
  return result;
}

BasisBitStream& BasisBitStream::operator&=(const BasisBitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] &= other.blocks[i];
  }

  return *this;
}

BasisBitStream& BasisBitStream::operator|=(const BasisBitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] |= other.blocks[i];
  }

  return *this;
}

BasisBitStream& BasisBitStream::operator^=(const BasisBitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] ^= other.blocks[i];
  }

  return *this;
}

BasisBitStream& BasisBitStream::operator>>=(const size_t offset) {
  for (auto& part : blocks) {
    part <<= offset;
  }
  return *this;
}

BasisBitStream& BasisBitStream::operator+=(const BasisBitStream& other) {
  assert(blocks.size() == other.blocks.size() && "sizes must be same");

  uint8_t carry = 0;
  for (auto i = 0; i < other.blocks.size(); ++i) {
    blocks[i] += other.blocks[i] + carry;
    carry = (blocks[i] >> bits_per_block) & 1;
    blocks[i] &= ~(1ULL << bits_per_block);
  }

  return *this;
}

void BasisBitStream::set(size_t pos, bool value) {
  if (value) {
    set(pos);
  } else {
    // TODO(me): clear
  }
}

void BasisBitStream::set(size_t pos) {
  auto [block_index, pos_in_block] = find_block(pos);
  blocks[block_index] |= 1ULL << pos_in_block;
}

void BasisBitStream::clear(size_t pos) {
  auto [block_index, pos_in_block] = find_block(pos);
  blocks[block_index] &= ~(1ULL << pos_in_block);
}

bool BasisBitStream::is_set(size_t pos) {
  auto [block_index, pos_in_block] = find_block(pos);
  return (blocks[block_index] >> pos_in_block) & 1;
}

std::pair<size_t, size_t> BasisBitStream::find_block(size_t pos) {
  auto part = pos / bits_per_block;
  auto pos_in_block = pos - (bits_per_block * part);
  return {part, pos_in_block};
}
