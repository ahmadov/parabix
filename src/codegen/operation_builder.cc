#include "codegen/operation_builder.h"

using OperationBuilder = codegen::OperationBuilder;

auto CLEAR_LAST_BIT = -9223372036854775809ULL; // ~(1ULL << 63)

void OperationBuilder::initializeGEPs() {
  auto cc_size = cc_list.size();
  cc_ptr.reserve(cc_size);
  marker_ptr.reserve(cc_size + 1);
  carry_ptr.reserve(cc_size);
  for (auto i = 0; i < cc_size; ++i) {
    cc_ptr[i] = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), cc_bit_stream, i, std::string("CC") + "_" + std::to_string(i));
    marker_ptr[i] = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), marker_bit_stream, i, std::string("M") + "_" + std::to_string(i));
    carry_ptr[i] = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), carry_bit_stream, i, std::string("CARRY") + "_" + std::to_string(i));
  }
  marker_ptr[cc_size] = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), marker_bit_stream, cc_size, std::string("M") + "_" + std::to_string(cc_size));
}

void OperationBuilder::codegen() {
  builder.CreateStore(builder.CreateLoad(builder.getInt64Ty(), cc_ptr[0]), marker_ptr[0]);
  for (size_t i = 0, end = cc_list.size(); i < end; ++i) {
    if (cc_list[i].isStar()) {
      buildMatchStar(i);
    } else {
      buildAdvance(i);
    }
  }
}

void OperationBuilder::buildAdvance(size_t index) {
  llvm::Value* M = builder.CreateLoad(builder.getInt64Ty(), marker_ptr[index]);
  llvm::Value* CC = builder.CreateLoad(builder.getInt64Ty(), cc_ptr[index]);
  llvm::Value* CARRY = builder.CreateLoad(builder.getInt64Ty(), carry_ptr[index]);

  auto result = M;
  result = builder.CreateAnd(result, CC);
  result = builder.CreateShl(result, 1);
  result = builder.CreateOr(result, CARRY);
  auto carry = builder.CreateAnd(builder.CreateLShr(result, 63), 1);
  result = builder.CreateAnd(result, CLEAR_LAST_BIT);

  builder.CreateStore(result, marker_ptr[index + 1]);
  builder.CreateStore(carry, carry_ptr[index]);
}

void OperationBuilder::buildMatchStar(size_t index) {
  llvm::Value* M = builder.CreateLoad(builder.getInt64Ty(), marker_ptr[index]);
  llvm::Value* CC = builder.CreateLoad(builder.getInt64Ty(), cc_ptr[index]);
  llvm::Value* CARRY = builder.CreateLoad(builder.getInt64Ty(), carry_ptr[index]);

  auto result = M;
  result = builder.CreateAnd(result, CC);
  result = builder.CreateAdd(result, builder.CreateAdd(CC, CARRY));
  result = builder.CreateXor(result, CC);
  result = builder.CreateOr(result, M);
  auto carry = builder.CreateAnd(builder.CreateLShr(result, 63), 1);
  result = builder.CreateAnd(result, CLEAR_LAST_BIT);

  builder.CreateStore(result, marker_ptr[index + 1]);
  builder.CreateStore(carry, carry_ptr[index]);
}
