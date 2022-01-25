#ifndef INCLUDE_CODEGEN_OPERATION_COMPILER_H_
#define INCLUDE_CODEGEN_OPERATION_COMPILER_H_

#include <algorithm>
#include <cstdint>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include "parser/cc.h"
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
      , runFnPtr(nullptr) {}

    void compile(const std::vector<parser::CC>& cc_list, bool verbose = false);

    void run(uint64_t* basis, uint64_t* cc, uint64_t* marker, uint64_t* carry);

    private:
    void compileRun(const std::vector<parser::CC>& cc_list);

    /// The llvm context.
    llvm::orc::ThreadSafeContext& context;
    /// The llvm module.
    std::unique_ptr<llvm::Module> module;
    /// The jit.
    JIT jit;
    /// The compiled match function.
    void (*runFnPtr)(uint64_t*, uint64_t*, uint64_t*, uint64_t*);
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_OPERATION_COMPILER_H_
