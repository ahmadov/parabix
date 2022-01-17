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
  for (auto i = 0; i < parts.size(); ++i) {
    result.parts[i] = ~parts[i];
  }
  return result;
}

BasisBitStream& BasisBitStream::operator&=(const BasisBitStream& other) {
  assert(parts.size() == other.parts.size() && "sizes must be same");

  for (auto i = 0; i < other.parts.size(); ++i) {
    parts[i] &= other.parts[i];
  }

  return *this;
}

BasisBitStream& BasisBitStream::operator|=(const BasisBitStream& other) {
  assert(parts.size() == other.parts.size() && "sizes must be same");

  for (auto i = 0; i < other.parts.size(); ++i) {
    parts[i] |= other.parts[i];
  }

  return *this;
}

BasisBitStream& BasisBitStream::operator^=(const BasisBitStream& other) {
  assert(parts.size() == other.parts.size() && "sizes must be same");

  for (auto i = 0; i < other.parts.size(); ++i) {
    parts[i] ^= other.parts[i];
  }

  return *this;
}

BasisBitStream& BasisBitStream::operator>>=(const size_t offset) {
  for (auto& part : parts) {
    part <<= offset;
  }
  return *this;
}

BasisBitStream& BasisBitStream::operator+=(const BasisBitStream& other) {
  assert(parts.size() == other.parts.size() && "sizes must be same");

  uint64_t carry = 0;
  for (auto i = 0; i < other.parts.size(); ++i) {
    parts[i] += other.parts[i] + carry;
    carry = (parts[i] >> BITS) & 1;
    parts[i] &= ~(1ULL << BITS);
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
  auto [part_index, pos_in_part] = find_part(pos);
  parts[part_index] |= 1ULL << pos_in_part;
}

void BasisBitStream::clear(size_t pos) {
  auto [part_index, pos_in_part] = find_part(pos);
  parts[part_index] &= ~(1ULL << pos_in_part);
}

bool BasisBitStream::is_set(size_t pos) {
  auto [part_index, pos_in_part] = find_part(pos);
  return (parts[part_index] >> pos_in_part) & 1;
}

std::pair<size_t, size_t> BasisBitStream::find_part(size_t pos) {
  auto part = pos / BITS;
  auto pos_in_part = pos - (BITS * part);
  return {part, pos_in_part};
}
