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
      std::string range;
      while (!forward_match(']')) {
        range.push_back(input_[pos_]);
      }
      assert(range.size() == 3);
      if (forward_match('*')) {
        cc_list_.emplace_back(CC(range[0], range[2], true));
        forward();
      } else {
        cc_list_.emplace_back(CC(range[0], range[2]));
      }
    } else { // single character
      char current = input_[pos_];
      if (forward_match('*')) {
        cc_list_.emplace_back(CC(current, true));
        forward();
      } else {
        cc_list_.emplace_back(CC(current));
      }
    }
  }
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
