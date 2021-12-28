#ifndef INCLUDE_CODEGEN_EXPRESSION_COMPILER_LLVM_H_
#define INCLUDE_CODEGEN_EXPRESSION_COMPILER_LLVM_H_

#include <algorithm>
#include <cstdint>

#include "codegen/ast.h"
#include "codegen/jit.h"

namespace codegen {

  class ExpressionCompiler {
    public:

    explicit ExpressionCompiler(llvm::orc::ThreadSafeContext& context)
      : context(context)
      , module(std::make_unique<llvm::Module>("expression_module", *context.getContext()))
      , jit(context)
      , fnPtr(nullptr) {}

    void compile(BitwiseExpression& expression, bool verbose = false);

    uint8_t run(uint8_t value);

    private:
    /// The llvm context.
    llvm::orc::ThreadSafeContext& context;
    /// The llvm module.
    std::unique_ptr<llvm::Module> module;
    /// The jit.
    JIT jit;
    /// The compiled function.
    uint8_t (*fnPtr)(uint8_t value);


  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_EXPRESSION_COMPILER_LLVM_H_
