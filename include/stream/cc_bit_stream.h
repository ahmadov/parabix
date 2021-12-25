// ---------------------------------------------------------------------------
#ifndef INCLUDE_STREAM_CC_BIT_STREAM_H_
#define INCLUDE_STREAM_CC_BIT_STREAM_H_
// ---------------------------------------------------------------------------
#include <ostream>
#include <sstream>
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
    std::stringstream out;
    out << "CC([";
    out << cc.low_;
    if (cc.low_ != cc.high_) {
      out << "-" << cc.high_;
    }
    out << "]";
    if (cc.star_) {
      out << "*";
    }
    out << ")";
    return os << out.str();
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
