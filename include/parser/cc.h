// ---------------------------------------------------------------------------
#ifndef INCLUDE_PARSER_CC_H_
#define INCLUDE_PARSER_CC_H_
// ---------------------------------------------------------------------------
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
namespace parser {
// ---------------------------------------------------------------------------
// Character Class Bit Stream
class CC {
  public:

  explicit CC(char low, char high, bool star = false)
    : star_(star)
    , low_(low)
    , high_(high) {}

  explicit CC(char character, bool star = false)
    : star_(star)
    , low_(character)
    , high_(character) {}

  [[nodiscard]] constexpr std::pair<char, char> getRange() const { return {low_, high_}; }

  [[nodiscard]] constexpr bool isStar() const { return star_; }

  friend std::ostream& operator<<(std::ostream& os, const CC& cc) {
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
} // namespace parser
// ---------------------------------------------------------------------------
#endif  // INCLUDE_PARSER_CC_H_
// ---------------------------------------------------------------------------
