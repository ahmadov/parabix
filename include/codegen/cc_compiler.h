#ifndef INCLUDE_CODEGEN_CC_COMPILER_H_
#define INCLUDE_CODEGEN_CC_COMPILER_H_

#include <algorithm>
#include <vector>

#include "codegen/ast.h"
#include "parser/cc.h"

const uint8_t SINGLE_CHAR_BITS = 255;

namespace codegen {

  class CCCompiler {
    public:

    std::unique_ptr<BitwiseExpression> compile(const parser::CC& cc);

    private:
    std::unique_ptr<BitwiseExpression> createSingleOrRange(std::pair<char, char>& range);

    std::unique_ptr<BitwiseExpression> createBitPattern(uint8_t pattern, uint8_t bits);

    std::unique_ptr<BitwiseExpression> createRange(uint8_t low, uint8_t high);

    std::unique_ptr<BitwiseExpression> createLERange(uint8_t bits, uint8_t n);

    std::unique_ptr<BitwiseExpression> createGERange(uint8_t bits, uint8_t n);

    std::unique_ptr<BitwiseExpression> createBoolean(bool value);

    std::unique_ptr<BitwiseExpression> createBit(unsigned bit);

    std::unique_ptr<BitwiseExpression> createSelection(
        std::unique_ptr<BitwiseExpression> if_expr,
        std::unique_ptr<BitwiseExpression> true_expr,
        std::unique_ptr<BitwiseExpression> false_expr
    );

    /// Create & (and) expression
    std::unique_ptr<BitwiseExpression> createAnd(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right);

    /// Create | (or) expression
    std::unique_ptr<BitwiseExpression> createOr(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right);

    /// Create ~ (not) expression
    std::unique_ptr<BitwiseExpression> createNot(std::unique_ptr<BitwiseExpression> expr);

    bool equal_expressions(BitwiseExpression* left, BitwiseExpression* right);
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_CC_COMPILER_H_
