#include "parser/re_parser.h"

using ReParser = parser::ReParser;
using CCBitStream = stream::CCBitStream;

void ReParser::parse() {
  while (!eof()) {
    if (match('[')) { // start a range
      std::string range;
      while (!forward_match(']')) {
        range.push_back(input_[pos_]);
      }
      if (forward_match('*')) {
        cc_list_.emplace_back(CCBitStream(range, true));
        forward();
      } else {
        cc_list_.emplace_back(CCBitStream(range));
      }
    } else { // single character
      char current = input_[pos_];
      if (forward_match('*')) {
        cc_list_.emplace_back(CCBitStream(current, true));
        forward();
      } else {
        cc_list_.emplace_back(CCBitStream(current));
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

const std::vector<CCBitStream>& ReParser::getCCs() {
  return cc_list_;
}
