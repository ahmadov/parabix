#include "codegen/expression_builder.h"
#include "codegen/parabix_compiler.h"

using ParabixCompiler = codegen::ParabixCompiler;

auto CLEAR_LAST_BIT = -9223372036854775809ULL; // ~(1ULL << 63)

void ParabixCompiler::compile(const std::vector<std::unique_ptr<BitwiseExpression>>& expressions, bool verbose) {
  compileMatch(expressions);
  compileAdvance();
  compileMatchStar();
  if (verbose) {
    module->print(llvm::errs(), nullptr);
  }
  auto error = jit.addModule(std::move(module));
  if (error) {
    throw std::runtime_error{"cannot add a module to JIT"};
  }
  matchFnPtr = reinterpret_cast<decltype(matchFnPtr)>(jit.getPointerToFunction("cc_match"));
  advanceFnPtr = reinterpret_cast<decltype(advanceFnPtr)>(jit.getPointerToFunction("advance"));
  matchStarFnPtr = reinterpret_cast<decltype(advanceFnPtr)>(jit.getPointerToFunction("match_star"));
}

void ParabixCompiler::compileMatch(const std::vector<std::unique_ptr<BitwiseExpression>>& expressions) {
  auto& ctx = *context.getContext();
  llvm::IRBuilder<> builder(ctx);

  // define void @cc_match(i64* %basis, i64* %cc) {
  auto matchT = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {llvm::Type::getInt64PtrTy(ctx), llvm::PointerType::getInt64PtrTy(ctx)}, false);
  auto matchFN = llvm::cast<llvm::Function>(module->getOrInsertFunction("cc_match", matchT).getCallee());

  // entry:
  llvm::BasicBlock *matchFnEntryBlock = llvm::BasicBlock::Create(ctx, "entry", matchFN);
  builder.SetInsertPoint(matchFnEntryBlock);

  std::vector<llvm::Value *> matchFnArgs;
  for(llvm::Function::arg_iterator ai = matchFN->arg_begin(), ae = matchFN->arg_end(); ai != ae; ++ai) {
      matchFnArgs.push_back(&*ai);
  }
  if(matchFnArgs.size() != 2) {
      throw std::runtime_error{"LLVM: cc_match() does not have enough arguments"};
  }

  ExpressionBuilder expression_builder(builder, matchFnArgs);
  llvm::Value* arr = matchFnArgs[1];

  for (size_t i = 0, end = expressions.size(); i < end; ++i) {
    auto* expr_value = expression_builder.codegen(expressions[i].get());
    auto* array_idx = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), arr, i, "arrayidx");
    builder.CreateStore(expr_value, array_idx);
  }
  builder.CreateRetVoid();
}

void ParabixCompiler::compileAdvance() {
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

void ParabixCompiler::compileMatchStar() {
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

void ParabixCompiler::runMatch(uint64_t* basis, uint64_t* cc) {
  if (matchFnPtr == nullptr) {
    throw std::runtime_error{"the cc_match method is not initialized."};
  }
  matchFnPtr(basis, cc);
}

ParabixCompiler::OperationResult ParabixCompiler::runAdvance(uint64_t marker, uint64_t cc, uint8_t carry) {
  if (advanceFnPtr == nullptr) {
    throw std::runtime_error{"the advance method is not initialized."};
  }
  return advanceFnPtr(marker, cc, carry);
}

ParabixCompiler::OperationResult ParabixCompiler::runMatchStar(uint64_t marker, uint64_t cc, uint8_t carry) {
  if (matchStarFnPtr == nullptr) {
    throw std::runtime_error{"the match_star method is not initialized."};
  }
  return matchStarFnPtr(marker, cc, carry);
}
