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
// Basis Bit Stream
class BasisBitStream {
  public:
    explicit BasisBitStream(size_t length) {
      parts.resize((length + 63) / 64, 0);
    }

    BasisBitStream() = default;
    BasisBitStream(BasisBitStream const&) = default;

    void set(size_t pos, bool value);

    [[nodiscard]] bool is_set(size_t pos);

    [[nodiscard]] size_t size() {
      return parts.size() * BITS;
    }

    [[nodiscard]] size_t size() const {
      return parts.size() * BITS;
    }

    std::vector<uint64_t> getParts() const {
      return parts;
    }

    BasisBitStream& operator=(const BasisBitStream& other) {
      if (this != &other) {
        parts = other.parts;
      }
      return *this;
    }

    BasisBitStream& operator=(BasisBitStream&& other) noexcept {
      if (this != &other) {
        parts = std::move(other.parts);
      }
      return *this;
    }

    BasisBitStream operator+(const BasisBitStream& other) const;

    BasisBitStream operator&(const BasisBitStream& other) const;

    BasisBitStream operator|(const BasisBitStream& other) const;

    BasisBitStream operator^(const BasisBitStream& other) const;

    BasisBitStream operator>>(const size_t offset) const;

    BasisBitStream operator~() const;

    BasisBitStream& operator+=(const BasisBitStream& other);

    BasisBitStream& operator&=(const BasisBitStream& other);

    BasisBitStream& operator|=(const BasisBitStream& other);

    BasisBitStream& operator^=(const BasisBitStream& other);

    BasisBitStream& operator>>=(const size_t offset);

    friend std::ostream& operator<<(std::ostream& os, const BasisBitStream& marker) {
      std::stringstream out;
      
      for (const auto& part : marker.parts) {
        for (auto i = 0; i < marker.BITS; ++i) {
          auto is_set = (part >> i) & 1;
          out << (is_set ? "1" : ".");
        }
      }

      return os << out.str();
    }

  protected:
    const size_t BITS = 63; // number of bits that each part stores

    void set(size_t pos);

    void clear(size_t pos);

    std::pair<size_t, size_t> find_part(size_t pos);

    std::vector<uint64_t> parts;
};
// ---------------------------------------------------------------------------
} // namespace stream
// ---------------------------------------------------------------------------
#endif  // INCLUDE_STREAM_BASIS_BIT_STREAM_H_
// ---------------------------------------------------------------------------
