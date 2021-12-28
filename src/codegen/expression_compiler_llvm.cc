#include "codegen/expression_compiler_llvm.h"

using ExpressionCompiler = codegen::ExpressionCompiler;
using BitwiseExpression = codegen::BitwiseExpression;

void ExpressionCompiler::compile(BitwiseExpression& expression, bool verbose) {
  auto& ctx = *context.getContext();
  llvm::IRBuilder<> builder(ctx);

  // define i8 @cc_match(i8 %val) {
  auto matchT = llvm::FunctionType::get(llvm::Type::getInt8Ty(ctx), {llvm::Type::getInt8Ty(ctx)}, false);
  auto matchFN = llvm::cast<llvm::Function>(module->getOrInsertFunction("cc_match", matchT).getCallee());

  // entry:
  llvm::BasicBlock *matchFnEntryBlock = llvm::BasicBlock::Create(ctx, "entry", matchFN);
  builder.SetInsertPoint(matchFnEntryBlock);

  std::vector<llvm::Value *> matchFnArgs;
  for(llvm::Function::arg_iterator ai = matchFN->arg_begin(), ae = matchFN->arg_end(); ai != ae; ++ai) {
      matchFnArgs.push_back(&*ai);
  }
  if(matchFnArgs.size() != 1) {
      throw("LLVM: match() does not have enough arguments");
  }

  builder.CreateRet(expression.codegen(builder, matchFnArgs[0]));

  if (verbose) {
    module->print(llvm::errs(), nullptr);
  }

  auto error = jit.addModule(std::move(module));
  if (error) {
    throw std::runtime_error{"cannot add a module to JIT"};
  }
  fnPtr = reinterpret_cast<decltype(fnPtr)>(jit.getPointerToFunction("cc_match"));
}

uint8_t ExpressionCompiler::run(uint8_t value) {
  return fnPtr(value);
}
