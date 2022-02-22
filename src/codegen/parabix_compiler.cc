#include "codegen/parabix_compiler.h"
#include "codegen/cc_compiler.h"
#include "codegen/expression_builder.h"
#include "codegen/operation_builder.h"

using ParabixCompiler = codegen::ParabixCompiler;
using ExpressionBuilder = codegen::ExpressionBuilder;
using OperationBuilder = codegen::OperationBuilder;
using CCCompiler = codegen::CCCompiler;

void ParabixCompiler::compile(const std::vector<parser::CC>& cc_list, bool verbose) {
  compileRun(cc_list);
  if (verbose) {
    module->print(llvm::errs(), nullptr);
  }
  auto error = jit.addModule(std::move(module));
  if (error) {
    throw std::runtime_error{"cannot add a module to JIT"};
  }
  runFnPtr = reinterpret_cast<decltype(runFnPtr)>(jit.getPointerToFunction("run"));
}

void ParabixCompiler::compileRun(const std::vector<parser::CC>& cc_list) {
  auto& ctx = *context.getContext();
  llvm::IRBuilder<> builder(ctx);

  // define void @run(i64* %basis, i64* %cc, i64* %marker, *i64 %carry) {
  auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {
      llvm::PointerType::getInt64PtrTy(ctx),
      llvm::PointerType::getInt64PtrTy(ctx),
      llvm::PointerType::getInt64PtrTy(ctx),
      llvm::PointerType::getInt64PtrTy(ctx)
    },
    false
  );
  auto func = llvm::cast<llvm::Function>(module->getOrInsertFunction("run", funcType).getCallee());

  // entry:
  llvm::BasicBlock *funcEntryBlock = llvm::BasicBlock::Create(ctx, "entry", func);
  builder.SetInsertPoint(funcEntryBlock);

  std::vector<llvm::Value *> funcArgs;
  for(llvm::Function::arg_iterator ai = func->arg_begin(), ae = func->arg_end(); ai != ae; ++ai) {
      funcArgs.push_back(&*ai);
  }
  if(funcArgs.size() != 4) {
      throw std::runtime_error{"LLVM: run() does not have enough arguments"};
  }
  auto basis = funcArgs[0];
  auto cc = funcArgs[1];
  auto marker = funcArgs[2];
  auto carry = funcArgs[3];

  CCCompiler cc_compiler;
  ExpressionBuilder expression_builder(builder, basis);
  OperationBuilder operation_builder(builder);

  for (size_t i = 0, end = cc_list.size(); i < end; ++i) {
    auto expression = cc_compiler.compile(cc_list[i]);
    auto* cc_value = expression_builder.codegen(expression.get());
    auto* cc_ptr = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), cc, i, "cc_ptr");
    builder.CreateStore(cc_value, cc_ptr);

    auto* marker_ptr = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), marker, i, "marker_ptr");
    auto* carry_ptr = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), carry, i, "carry_ptr");
    if (i == 0) {
      builder.CreateStore(cc_value, marker_ptr);
    }

    auto marker_value = builder.CreateLoad(builder.getInt64Ty(), marker_ptr);
    auto carry_value = builder.CreateLoad(builder.getInt64Ty(), carry_ptr);
    auto [next_marker, next_carry] = operation_builder.codegen(cc_list[i], cc_value, marker_value, carry_value);

    auto* next_marker_ptr = builder.CreateConstInBoundsGEP1_64(builder.getInt64Ty(), marker, i + 1, "next_marker_ptr");
    builder.CreateStore(next_marker, next_marker_ptr);
    builder.CreateStore(next_carry, carry_ptr);
  }

  builder.CreateRetVoid();
}

void ParabixCompiler::run(uint64_t* basis, uint64_t* cc, uint64_t* marker, uint64_t* carry) {
  if (runFnPtr == nullptr) {
    throw std::runtime_error{"the run method is not initialized."};
  }
  runFnPtr(basis, cc, marker, carry);
}
