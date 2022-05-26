// ---------------------------------------------------------------------------
#ifndef INCLUDE_PARSER_CC_H_
#define INCLUDE_PARSER_CC_H_
// ---------------------------------------------------------------------------
#include <vector>
#include <ostream>
#include <sstream>
#include <string>
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
namespace parser {
// ---------------------------------------------------------------------------
// Character Class Bit Stream
class CC {
  public:

    explicit CC(std::vector<std::pair<char, char>> ranges, bool star = false)
      : ranges(std::move(ranges))
      , star_(star) {}

    [[nodiscard]] std::vector<std::pair<char, char>> getRanges() const { return ranges; }

    [[nodiscard]] constexpr bool isStar() const { return star_; }

    [[nodiscard]] bool match(char c) {
      for (auto& range : ranges) {
        if (c >= range.first && c <= range.second) {
          return true;
        }
      }
      return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const CC& cc) {
      std::stringstream out;
      out << "CC([";
      for (auto& [low, high] : cc.ranges) {
        out << low;
        if (low != high) {
          out << "-" << high;
        }
      }
      out << "]";
      if (cc.star_) {
        out << "*";
      }
      out << ")";
      return os << out.str();
    }

  private:
    std::vector<std::pair<char, char>> ranges;
    bool star_;
};
// ---------------------------------------------------------------------------
} // namespace parser
// ---------------------------------------------------------------------------
#endif  // INCLUDE_PARSER_CC_H_
// ---------------------------------------------------------------------------
