// ---------------------------------------------------------------------------
#ifndef INCLUDE_PARSER_RE_PARSER_H_
#define INCLUDE_PARSER_RE_PARSER_H_
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

#include "stream/cc_bit_stream.h"
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
namespace parser {
// ---------------------------------------------------------------------------
// Very simple regex cc parser
class ReParser {
  public:

    const std::vector<stream::CCBitStream>& parse(const char* input);

  private:
    void parse();
    bool eof();
    void forward();
    bool match(char);
    bool forward_match(char);

    size_t pos_;
    std::vector<stream::CCBitStream> cc_list_;
    std::string_view input_;
};
// ---------------------------------------------------------------------------
} // namespace parser
// ---------------------------------------------------------------------------
#endif  // INCLUDE_PARSER_RE_PARSER_H_
// ---------------------------------------------------------------------------
