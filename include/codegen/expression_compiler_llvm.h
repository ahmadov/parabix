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

    void compile(const std::vector<std::unique_ptr<BitwiseExpression>>& expressions, bool verbose = false);

    void run(uint64_t* basis, uint64_t* cc);

    private:
    /// The llvm context.
    llvm::orc::ThreadSafeContext& context;
    /// The llvm module.
    std::unique_ptr<llvm::Module> module;
    /// The jit.
    JIT jit;
    /// The compiled function.
    void (*fnPtr)(uint64_t*, uint64_t*);

  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_EXPRESSION_COMPILER_LLVM_H_
