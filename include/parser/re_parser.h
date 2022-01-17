// ---------------------------------------------------------------------------
#ifndef INCLUDE_PARSER_RE_PARSER_H_
#define INCLUDE_PARSER_RE_PARSER_H_
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

#include "parser/cc.h"
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
namespace parser {
// ---------------------------------------------------------------------------
// Very simple regex cc parser
class ReParser {
  public:

    const std::vector<parser::CC>& parse(const char* input);

  private:
    void parse();
    bool eof();
    void forward();
    bool match(char);
    bool forward_match(char);
    std::vector<std::pair<char, char>> parseRanges(); 

    size_t pos_;
    std::vector<parser::CC> cc_list_;
    std::string_view input_;
};
// ---------------------------------------------------------------------------
} // namespace parser
// ---------------------------------------------------------------------------
#endif  // INCLUDE_PARSER_RE_PARSER_H_
// ---------------------------------------------------------------------------
