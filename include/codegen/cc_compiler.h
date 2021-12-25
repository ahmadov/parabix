#ifndef INCLUDE_CODEGEN_CC_COMPILER_H_
#define INCLUDE_CODEGEN_CC_COMPILER_H_

#include <vector>

#include "codegen/ast.h"
#include "stream/cc_bit_stream.h"

const uint8_t SINGLE_CHAR_BITS = 255;

namespace codegen {

  class CCCompiler {
    public:
    void compile(const std::vector<stream::CCBitStream>& cc_list);

    private:
    std::unique_ptr<BitwiseExpression> createBitPattern(uint8_t pattern, uint8_t bits);

    std::unique_ptr<Bit> createBit(unsigned bit);

    /// Create & (and) expression
    std::unique_ptr<AndExpression> createAnd(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right);

    /// Create | (or) expression
    std::unique_ptr<OrExpression> createOr(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right);

    /// Create ^ (xor) expression
    std::unique_ptr<XorExpression> createXor(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right);

    /// Create ~ (not) expression
    std::unique_ptr<NotExpression> createNot(std::unique_ptr<BitwiseExpression> expr);
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_CC_COMPILER_H_
