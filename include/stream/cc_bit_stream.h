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

  explicit CCBitStream(char low, char high, bool star = false)
    : star_(star)
    , low_(low)
    , high_(high) {}

  explicit CCBitStream(char character, bool star = false)
    : star_(star)
    , low_(character)
    , high_(character) {}

  [[nodiscard]] constexpr std::pair<char, char> getRange() const { return {low_, high_}; }

  friend std::ostream& operator<<(std::ostream& os, const CCBitStream& cc) {
    os << "CC([";
    os << cc.low_;
    if (cc.low_ != cc.high_) {
      os << "-" << cc.high_;
    }
    os << "]";
    if (cc.star_) {
      os << "*";
    }
    os << ")";
    return os;
  }

  private:
    bool star_;
    char low_;
    char high_;
};
// ---------------------------------------------------------------------------
} // namespace stream
// ---------------------------------------------------------------------------
#endif  // INCLUDE_STREAM_CC_BIT_STREAM_H_
// ---------------------------------------------------------------------------
