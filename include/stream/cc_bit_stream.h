// ---------------------------------------------------------------------------
#ifndef INCLUDE_STREAM_CC_BIT_STREAM_H_
#define INCLUDE_STREAM_CC_BIT_STREAM_H_
// ---------------------------------------------------------------------------
#include <ostream>
#include <string>
#include <string_view>
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
namespace stream {
// ---------------------------------------------------------------------------
// Character Class Bit Stream
class CCBitStream {
  public:
  enum class Type {
    SINGLE,
    SINGLE_STAR,
    RANGE,
    RANGE_STAR,
  };

  explicit CCBitStream(char character, bool star = false)
    : type_(star ? Type::SINGLE_STAR : Type::SINGLE)
    , character_(character) {}

  explicit CCBitStream(std::string& range, bool star = false)
    : type_(star ? Type::RANGE_STAR : Type::RANGE)
    , range_(range) {}

  [[nodiscard]] constexpr Type getType() const { return type_; }

  [[nodiscard]] constexpr char getChar() const { return character_; }

  friend std::ostream& operator<<(std::ostream& os, const CCBitStream& cc) {
    os << "CC([";
    if (cc.type_ == Type::RANGE || cc.type_ == Type::RANGE_STAR) {
      os << cc.range_;
    } else {
      os << cc.character_;
    }
    os << "]";
    if (cc.type_ == Type::SINGLE_STAR || cc.type_ == Type::RANGE_STAR) {
      os << "*";
    }
    os << ")";
    return os;
  }

  private:
    Type type_;

    char character_;      // Type::SINGLE
    std::string range_;   // Type::RANGE
};
// ---------------------------------------------------------------------------
} // namespace stream
// ---------------------------------------------------------------------------
#endif  // INCLUDE_STREAM_CC_BIT_STREAM_H_
// ---------------------------------------------------------------------------
