#ifndef INCLUDE_CODEGEN_OPERATION_COMPILER_H_
#define INCLUDE_CODEGEN_OPERATION_COMPILER_H_

#include <algorithm>
#include <cstdint>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include "codegen/ast.h"
#include "codegen/jit.h"

namespace codegen {

  class ParabixCompiler {
    public:

    struct OperationResult {
      uint64_t marker;
      uint8_t carry;
    };

    explicit ParabixCompiler(llvm::orc::ThreadSafeContext& context)
      : context(context)
      , module(std::make_unique<llvm::Module>("parabix_module", *context.getContext()))
      , jit(context)
      , matchFnPtr(nullptr)
      , advanceFnPtr(nullptr)
      , matchStarFnPtr(nullptr) {}

    void compile(const std::vector<std::unique_ptr<BitwiseExpression>>& expressions, bool verbose = false);

    void runMatch(uint64_t* basis, uint64_t* cc);

    OperationResult runAdvance(uint64_t marker, uint64_t cc, uint8_t carry);

    OperationResult runMatchStar(uint64_t marker, uint64_t cc, uint8_t carry);

    private:
    void compileMatch(const std::vector<std::unique_ptr<BitwiseExpression>>& expressions);

    void compileAdvance();

    void compileMatchStar();

    /// The llvm context.
    llvm::orc::ThreadSafeContext& context;
    /// The llvm module.
    std::unique_ptr<llvm::Module> module;
    /// The jit.
    JIT jit;
    /// The compiled match function.
    void (*matchFnPtr)(uint64_t*, uint64_t*);
    /// The compiled advance function.
    OperationResult (*advanceFnPtr)(uint64_t, uint64_t, uint8_t);
    /// The compiled match_star function.
    OperationResult (*matchStarFnPtr)(uint64_t, uint64_t, uint8_t);
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_OPERATION_COMPILER_H_
