#include "parser/re_parser.h"
#include <cassert>

using ReParser = parser::ReParser;
using CC = parser::CC;

const std::vector<CC>& ReParser::parse(const char* input) {
  input_ = input;
  pos_ = 0;
  parse();
  return cc_list_;
}

void ReParser::parse() {
  while (!eof()) {
    if (match('[')) { // start a range
      forward();
      auto ranges = parseRanges();
      if (forward_match('*')) {
        cc_list_.emplace_back(CC(ranges, true));
        forward();
      } else {
        cc_list_.emplace_back(CC(ranges));
      }
    } else { // single character
      char current = input_[pos_];
      if (forward_match('*')) {
        cc_list_.emplace_back(CC({ {current, current} }, true));
        forward();
      } else {
        cc_list_.emplace_back(CC({ {current, current} }));
      }
    }
  }
}

std::vector<std::pair<char, char>> ReParser::parseRanges() {
  std::vector<std::pair<char, char>> result;

  do {
    std::pair<char, char> range;
    range.first = input_[pos_];
    assert(forward_match('-')); forward();
    range.second = input_[pos_];
    result.emplace_back(range);
  } while (!forward_match(']'));

  return result;
}

void ReParser::forward() {
  if (!eof()) {
    ++pos_;
  }
}

bool ReParser::match(char c) {
  return !eof() && input_[pos_] == c;
}

bool ReParser::forward_match(char c) {
  forward();
  return match(c);
}

bool ReParser::eof() {
  return input_[pos_] == '\0';
}
