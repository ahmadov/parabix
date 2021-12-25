#include "codegen/cc_compiler.h"
#include <algorithm>
#include <iostream>

using CCCompiler = codegen::CCCompiler;
using Bit = codegen::Bit;
using BitwiseExpression = codegen::BitwiseExpression;
using AndExpression = codegen::AndExpression;
using OrExpression = codegen::OrExpression;
using XorExpression = codegen::XorExpression;
using NotExpression = codegen::NotExpression;
using CCBitStream = stream::CCBitStream;

void CCCompiler::compile(const std::vector<CCBitStream>& cc_list) {
  for (auto& cc : cc_list) {
    if (cc.getType() == CCBitStream::Type::SINGLE || cc.getType() == CCBitStream::Type::SINGLE_STAR) {
      auto expression = createBitPattern(cc.getChar(), SINGLE_CHAR_BITS);
      std::cout << "single character expression = " << expression->as_string() << std::endl;
    } else {
      // TODO(me): make range
    }
  }
}

std::unique_ptr<BitwiseExpression> CCCompiler::createBitPattern(uint8_t pattern, uint8_t bits) {
  unsigned bit = 0;
  std::vector<std::unique_ptr<BitwiseExpression>> expressions;
  for (unsigned bit = 0; bits; ++bit) {
    auto test_bit = 1 << bit;
    if (bits & test_bit) {
      if (pattern & test_bit) {
        expressions.emplace_back(createBit(bit));
      } else {
        expressions.emplace_back(createNot(createBit(bit)));
      }
    }
    bits &= ~test_bit;
  }
  std::reverse(expressions.begin(), expressions.end());
  std::vector<std::unique_ptr<BitwiseExpression>> new_expressions;
  while (expressions.size() > 1) {
    for (size_t i = 0, end = expressions.size() / 2; i < end; ++i) {
      new_expressions.emplace_back(createAnd(
        std::move(expressions[2 * i]),
        std::move(expressions[2 * i + 1])
      ));
    }
    if (expressions.size() % 2) {
      new_expressions.emplace_back(std::move(expressions.back()));
    }
    expressions = std::move(new_expressions);
    new_expressions.clear();
  }
  return std::move(expressions[0]);
}

std::unique_ptr<Bit> CCCompiler::createBit(unsigned bit) {
  return std::make_unique<Bit>(bit);
}

std::unique_ptr<AndExpression> CCCompiler::createAnd(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right) {
  return std::make_unique<AndExpression>(std::move(left), std::move(right));
}

std::unique_ptr<OrExpression> CCCompiler::createOr(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right) {
  return std::make_unique<OrExpression>(std::move(left), std::move(right));
}

std::unique_ptr<XorExpression> CCCompiler::createXor(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right) {
  return std::make_unique<XorExpression>(std::move(left), std::move(right));
}

std::unique_ptr<NotExpression> CCCompiler::createNot(std::unique_ptr<BitwiseExpression> expr) {
  return std::make_unique<NotExpression>(std::move(expr));
}
