#include "codegen/cc_compiler.h"
#include <algorithm>
#include <iostream>

using CCCompiler = codegen::CCCompiler;
using Bit = codegen::Bit;
using True = codegen::True;
using False = codegen::False;
using BitwiseExpression = codegen::BitwiseExpression;
using AndExpression = codegen::AndExpression;
using OrExpression = codegen::OrExpression;
using NotExpression = codegen::NotExpression;
using CCBitStream = stream::CCBitStream;
using Type = BitwiseExpression::Type;

void CCCompiler::compile(const std::vector<CCBitStream>& cc_list) {
  for (auto& cc : cc_list) {
    auto [low, high] = cc.getRange();
    if (low == high) {
      auto expression = createBitPattern(low, SINGLE_CHAR_BITS);
      std::cout << "single character expression = " << expression->as_string() << std::endl;
    } else {
      auto expression = createRange(low, high);
      std::cout << "range expression            = " << expression->as_string() << std::endl;
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

std::unique_ptr<BitwiseExpression> CCCompiler::createRange(uint8_t low, uint8_t high) {
  uint8_t count = 0;
  for (uint8_t bits = low ^ high; bits; bits >>=1, ++count);
  uint8_t mask = (1 << count) - 1;
  auto common_part = createBitPattern(low & ~mask, SINGLE_CHAR_BITS ^ mask);
  if (count == 0) {
    // single character?
    return std::move(common_part);
  }
  auto low_part = createGERange(count - 1, low & mask);
  auto high_part = createLERange(count - 1, high & mask);
  return createAnd(std::move(common_part), createSelection( createBit(count - 1), std::move(high_part), std::move(low_part) ));
}

std::unique_ptr<BitwiseExpression> CCCompiler::createLERange(uint8_t bits, uint8_t n) {
  if (n + 1 == 1 << bits) {
    return createBoolean(true);
  }
  return createNot(createGERange(bits, n + 1));
}

std::unique_ptr<BitwiseExpression> CCCompiler::createGERange(uint8_t bits, uint8_t n) {
  if (bits == 0) {
    return createBoolean(true);
  }
  if (bits % 2 == 0 && (n >> (bits - 2)) == 0) {
    return createOr(
      createOr(createBit(bits - 1), createBit(bits - 2)),
      createGERange(bits - 2, n)
    );
  } else if (bits % 2 == 0 && (n >> (bits - 2)) == 3) {
    return createAnd(
      createAnd(createBit(bits - 1), createBit(bits - 2)),
      createGERange(bits - 2, n - (3 << (bits - 2)))
    );
  }
  auto high_bit = n & (1 << (bits - 1));
  auto low_bit = n - high_bit;
  auto low_range = createGERange(bits - 1, low_bit);
  if (high_bit == 0) {
    return createOr(createBit(bits - 1), std::move(low_range));
  } else {
    return createAnd(createBit(bits - 1), std::move(low_range));
  }
}

std::unique_ptr<BitwiseExpression> CCCompiler::createBit(unsigned bit) {
  return std::make_unique<Bit>(bit);
}

std::unique_ptr<BitwiseExpression> CCCompiler::createBoolean(bool value) {
  if (value) {
    return std::make_unique<True>();
  }
  return std::make_unique<False>();
}

std::unique_ptr<BitwiseExpression> CCCompiler::createSelection(
    std::unique_ptr<BitwiseExpression> if_expr,
    std::unique_ptr<BitwiseExpression> true_expr,
    std::unique_ptr<BitwiseExpression> false_expr
) {
  if (if_expr->getType() == Type::True) {
    return std::move(true_expr);
  } else if (if_expr->getType() == Type::False) {
    return std::move(false_expr);
  } else if (true_expr->getType() == Type::True) {
    return createOr(std::move(if_expr), std::move(false_expr));
  } else if (true_expr->getType() == Type::False) {
    return createAnd(createNot(std::move(if_expr)), std::move(false_expr));
  } else if (false_expr->getType() == Type::False) {
    return createAnd(std::move(if_expr), std::move(true_expr));
  } else if (false_expr->getType() == Type::True) {
    return createOr(createNot(std::move(if_expr)), std::move(true_expr));
  }
  throw std::runtime_error{"not implemented"};
}

std::unique_ptr<BitwiseExpression> CCCompiler::createAnd(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right) {
  if (left->getType() == Type::True) {
    return std::move(right);
  }
  if (right->getType() == Type::True) {
    return std::move(left);
  }
  if (left->getType() == Type::False || right->getType() == Type::False) {
    return createBoolean(false);
  }
  if (left->getType() == Type::Not && right->getType() == Type::Not) {
    return createNot(createOr(
      std::move(dynamic_cast<NotExpression*>(left.get())->child),
      std::move(dynamic_cast<NotExpression*>(right.get())->child)
    ));
  }
  return std::make_unique<AndExpression>(std::move(left), std::move(right));
}

std::unique_ptr<BitwiseExpression> CCCompiler::createOr(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right) {
  if (left->getType() == Type::True || right->getType() == Type::True) {
    return createBoolean(true);
  }
  if (left->getType() == Type::False) {
    return std::move(right);
  }
  if (right->getType() == Type::False) {
    return std::move(left);
  }
  if (left->getType() == Type::Not) {
    return createNot(createAnd(
      std::move(dynamic_cast<NotExpression*>(left.get())->child),
      createNot(std::move(right))
    ));
  }
  if (right->getType() == Type::Not) {
    return createNot(createAnd(
      createNot(std::move(left)),
      std::move(dynamic_cast<NotExpression*>(right.get())->child)
    ));
  }
  return std::make_unique<OrExpression>(std::move(left), std::move(right));
}

std::unique_ptr<BitwiseExpression> CCCompiler::createNot(std::unique_ptr<BitwiseExpression> expr) {
  if (expr->getType() == Type::True) {
    return createBoolean(false);
  }
  if (expr->getType() == Type::False) {
    return createBoolean(true);
  }
  if (expr->getType() == Type::Not) {
    return std::move(dynamic_cast<NotExpression*>(expr.get())->child);
  }
  return std::make_unique<NotExpression>(std::move(expr));
}
