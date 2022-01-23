#include "codegen/operation_compiler.h"

using OperationCompiler = codegen::OperationCompiler;

auto CLEAR_LAST_BIT = -9223372036854775809ULL; // ~(1ULL << 63)

void OperationCompiler::initialize(bool verbose) {
  compileAdvance();
  compileMatchStar();
  if (verbose) {
    module->print(llvm::errs(), nullptr);
  }
  auto error = jit.addModule(std::move(module));
  if (error) {
    throw std::runtime_error{"cannot add a module to JIT"};
  }
  advanceFnPtr = reinterpret_cast<decltype(advanceFnPtr)>(jit.getPointerToFunction("advance"));
  matchStarFnPtr = reinterpret_cast<decltype(advanceFnPtr)>(jit.getPointerToFunction("match_star"));
}

void OperationCompiler::compileAdvance() {
  auto& ctx = *context.getContext();
  llvm::IRBuilder<> builder(ctx);

  // define {i64, i8} @advance(i64 %marker, i64 %cc, i8 %carry) {
  auto returnType = llvm::StructType::get(ctx, {llvm::Type::getInt64Ty(ctx), llvm::Type::getInt8Ty(ctx)});
  auto funcType = llvm::FunctionType::get(returnType, {llvm::Type::getInt64Ty(ctx), llvm::Type::getInt64Ty(ctx), llvm::Type::getInt8Ty(ctx)}, false);
  auto func = llvm::cast<llvm::Function>(module->getOrInsertFunction("advance", funcType).getCallee());

  // entry:
  llvm::BasicBlock *funcEntryBlock = llvm::BasicBlock::Create(ctx, "entry", func);
  builder.SetInsertPoint(funcEntryBlock);

  std::vector<llvm::Value *> funcArgs;
  for(llvm::Function::arg_iterator ai = func->arg_begin(), ae = func->arg_end(); ai != ae; ++ai) {
      funcArgs.push_back(&*ai);
  }
  if(funcArgs.size() != 3) {
      throw std::runtime_error{"LLVM: advance() does not have enough arguments"};
  }

  auto returnValue = builder.CreateAlloca(returnType);

  auto marker = funcArgs[0];
  auto cc = funcArgs[1];
  auto carry = builder.CreateZExt(funcArgs[2], builder.getInt64Ty());

  auto result = marker;
  result = builder.CreateAnd(result, cc);
  result = builder.CreateShl(result, 1);
  result = builder.CreateOr(result, carry);
  carry = builder.CreateAnd(builder.CreateLShr(result, 63), 1);
  result = builder.CreateAnd(result, CLEAR_LAST_BIT);

  auto rm = builder.CreateStructGEP(returnType, returnValue, 0, "marker");
  builder.CreateStore(result, rm);

  auto rc = builder.CreateStructGEP(returnType, returnValue, 1, "carry");
  builder.CreateStore(builder.CreateBitCast(carry, builder.getInt8Ty()), rc);

  builder.CreateRet(builder.CreateLoad(returnType, returnValue));
}

void OperationCompiler::compileMatchStar() {
  auto& ctx = *context.getContext();
  llvm::IRBuilder<> builder(ctx);

  // define {i64, i8} @match_star(i64 %marker, i64 %cc, i8 %carry) {
  auto returnType = llvm::StructType::get(ctx, {llvm::Type::getInt64Ty(ctx), llvm::Type::getInt8Ty(ctx)});
  auto funcType = llvm::FunctionType::get(returnType, {llvm::Type::getInt64Ty(ctx), llvm::Type::getInt64Ty(ctx), llvm::Type::getInt8Ty(ctx)}, false);
  auto func = llvm::cast<llvm::Function>(module->getOrInsertFunction("match_star", funcType).getCallee());

  // entry:
  llvm::BasicBlock *funcEntryBlock = llvm::BasicBlock::Create(ctx, "entry", func);
  builder.SetInsertPoint(funcEntryBlock);

  std::vector<llvm::Value *> funcArgs;
  for(llvm::Function::arg_iterator ai = func->arg_begin(), ae = func->arg_end(); ai != ae; ++ai) {
      funcArgs.push_back(&*ai);
  }
  if(funcArgs.size() != 3) {
      throw std::runtime_error{"LLVM: match_star() does not have enough arguments"};
  }

  auto returnValue = builder.CreateAlloca(returnType);

  auto marker = funcArgs[0];
  auto cc = funcArgs[1];
  auto carry = builder.CreateZExt(funcArgs[2], builder.getInt64Ty());

  auto result = marker;
  result = builder.CreateAnd(result, cc);
  result = builder.CreateAdd(result, builder.CreateAdd(cc, carry));
  result = builder.CreateXor(result, cc);
  result = builder.CreateOr(result, marker);
  carry = builder.CreateAnd(builder.CreateLShr(result, 63), 1);
  result = builder.CreateAnd(result, CLEAR_LAST_BIT);

  auto rm = builder.CreateStructGEP(returnType, returnValue, 0);
  builder.CreateStore(result, rm);

  auto rc = builder.CreateStructGEP(returnType, returnValue, 1);
  builder.CreateStore(builder.CreateBitCast(carry, builder.getInt8Ty()), rc);

  builder.CreateRet(builder.CreateLoad(returnType, returnValue));
}

OperationCompiler::OperationResult OperationCompiler::runAdvance(uint64_t marker, uint64_t cc, uint8_t carry) {
  if (advanceFnPtr == nullptr) {
    throw std::runtime_error{"the advance method is not initialized."};
  }
  return advanceFnPtr(marker, cc, carry);
}

OperationCompiler::OperationResult OperationCompiler::runMatchStar(uint64_t marker, uint64_t cc, uint8_t carry) {
  if (matchStarFnPtr == nullptr) {
    throw std::runtime_error{"the match_star method is not initialized."};
  }
  return matchStarFnPtr(marker, cc, carry);
}
