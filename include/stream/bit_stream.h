// ---------------------------------------------------------------------------
#ifndef INCLUDE_STREAM_BASIS_BIT_STREAM_H_
#define INCLUDE_STREAM_BASIS_BIT_STREAM_H_
// ---------------------------------------------------------------------------
#include <vector>
#include <sstream>
#include <ostream>
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
namespace stream {
// ---------------------------------------------------------------------------
// Bit Stream
class BitStream {

  using block_type = uint64_t;
  const static size_t bits = sizeof(block_type) * 8;
  const static size_t bits_per_block = bits - 1; // number of bits that each block stores

  public:
    explicit BitStream(size_t length) {
      blocks.resize((length + bits - 1) / bits, 0);
    }

    BitStream() = default;
    BitStream(BitStream const&) = default;

    void set(size_t pos, bool value);

    [[nodiscard]] bool is_set(size_t pos);

    [[nodiscard]] size_t size() {
      return blocks.size() * bits_per_block;
    }

    [[nodiscard]] size_t size() const {
      return blocks.size() * bits_per_block;
    }

    BitStream& operator=(const BitStream& other) {
      if (this != &other) {
        blocks = other.blocks;
      }
      return *this;
    }

    BitStream& operator=(BitStream&& other) noexcept {
      if (this != &other) {
        blocks = std::move(other.blocks);
      }
      return *this;
    }

    BitStream operator+(const BitStream& other) const;

    BitStream operator&(const BitStream& other) const;

    BitStream operator|(const BitStream& other) const;

    BitStream operator^(const BitStream& other) const;

    BitStream operator>>(const size_t offset) const;

    BitStream operator~() const;

    BitStream& operator+=(const BitStream& other);

    BitStream& operator&=(const BitStream& other);

    BitStream& operator|=(const BitStream& other);

    BitStream& operator^=(const BitStream& other);

    BitStream& operator>>=(const size_t offset);

    friend std::ostream& operator<<(std::ostream& os, const BitStream& marker) {
      std::stringstream out;
      
      for (const auto& block : marker.blocks) {
        for (auto i = 0; i < marker.bits_per_block; ++i) {
          auto is_set = (block >> i) & 1;
          out << (is_set ? "1" : ".");
        }
      }

      return os << out.str();
    }

  protected:
    void set(size_t pos);

    void clear(size_t pos);

    std::pair<size_t, size_t> find_block(size_t pos);

    std::vector<block_type> blocks;
};
// ---------------------------------------------------------------------------
} // namespace stream
// ---------------------------------------------------------------------------
#endif  // INCLUDE_STREAM_BASIS_BIT_STREAM_H_
// ---------------------------------------------------------------------------
