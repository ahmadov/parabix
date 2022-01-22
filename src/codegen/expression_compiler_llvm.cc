#include "codegen/expression_builder.h"
#include "codegen/expression_compiler_llvm.h"

using ExpressionCompiler = codegen::ExpressionCompiler;
using ExpressionBuilder = codegen::ExpressionBuilder;
using BitwiseExpression = codegen::BitwiseExpression;

void ExpressionCompiler::compile(const std::vector<std::unique_ptr<BitwiseExpression>>& expressions, bool verbose) {
  auto& ctx = *context.getContext();
  llvm::IRBuilder<> builder(ctx);

  // define vec i8 @cc_match(i8 %val) {
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
      throw std::runtime_error{"LLVM: foo() does not have enough arguments"};
  }

  ExpressionBuilder expression_builder(builder, matchFnArgs);
  llvm::Value* arr = matchFnArgs[1];

  for (size_t i = 0, end = expressions.size(); i < end; ++i) {
    auto* expr_value = expression_builder.codegen(expressions[i].get());
    auto* array_idx = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), arr, i, "arrayidx");
    builder.CreateStore(expr_value, array_idx);
  }
  builder.CreateRetVoid();

  if (verbose) {
    module->print(llvm::errs(), nullptr);
  }

  auto error = jit.addModule(std::move(module));
  if (error) {
    throw std::runtime_error{"cannot add a module to JIT"};
  }
  fnPtr = reinterpret_cast<decltype(fnPtr)>(jit.getPointerToFunction("cc_match"));
}

void ExpressionCompiler::run(uint64_t* basis, uint64_t* cc) {
  return fnPtr(basis, cc);
}
