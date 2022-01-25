#include "codegen/operation_builder.h"

using OperationBuilder = codegen::OperationBuilder;

auto CLEAR_LAST_BIT = -9223372036854775809ULL; // ~(1ULL << 63)

std::pair<llvm::Value*, llvm::Value*> OperationBuilder::codegen(const parser::CC& cc, llvm::Value* cc_bit_stream, llvm::Value* marker_bit_stream, llvm::Value* carry) {
  if (cc.isStar()) {
    return buildMatchStar(cc_bit_stream, marker_bit_stream, carry);
  }
  return buildAdvance(cc_bit_stream, marker_bit_stream, carry);
}

std::pair<llvm::Value*, llvm::Value*> OperationBuilder::buildAdvance(llvm::Value* CC, llvm::Value* M, llvm::Value* CARRY) {
  auto result_bit_stream = M;
  result_bit_stream = builder.CreateAnd(result_bit_stream, CC);
  result_bit_stream = builder.CreateShl(result_bit_stream, 1);
  result_bit_stream = builder.CreateOr(result_bit_stream, CARRY);
  auto carry = builder.CreateAnd(builder.CreateLShr(result_bit_stream, 63), 1);
  result_bit_stream = builder.CreateAnd(result_bit_stream, CLEAR_LAST_BIT);

  return {result_bit_stream, carry};
}

std::pair<llvm::Value*, llvm::Value*> OperationBuilder::buildMatchStar(llvm::Value* CC, llvm::Value* M, llvm::Value* CARRY) {
  auto result_bit_stream = M;
  result_bit_stream = builder.CreateAnd(result_bit_stream, CC);
  result_bit_stream = builder.CreateAdd(result_bit_stream, builder.CreateAdd(CC, CARRY));
  result_bit_stream = builder.CreateXor(result_bit_stream, CC);
  result_bit_stream = builder.CreateOr(result_bit_stream, M);
  auto carry = builder.CreateAnd(builder.CreateLShr(result_bit_stream, 63), 1);
  result_bit_stream = builder.CreateAnd(result_bit_stream, CLEAR_LAST_BIT);

  return {result_bit_stream, carry};
}
