#ifndef INCLUDE_CODEGEN_OPERATION_BUILDER_H_
#define INCLUDE_CODEGEN_OPERATION_BUILDER_H_

#include <vector>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/ErrorHandling.h>
#include "parser/cc.h"

namespace codegen {

  class OperationBuilder {
    public:

      explicit OperationBuilder(llvm::IRBuilder<>& builder)
        : builder(builder) {}

      std::pair<llvm::Value*, llvm::Value*>  codegen(const parser::CC& cc, llvm::Value* cc_bit_stream, llvm::Value* marker_bit_stream, llvm::Value* carry);

    private:
      std::pair<llvm::Value*, llvm::Value*> buildAdvance(llvm::Value* CC, llvm::Value* M, llvm::Value* CARRY);

      std::pair<llvm::Value*, llvm::Value*> buildMatchStar(llvm::Value* CC, llvm::Value* M, llvm::Value* CARRY);

      llvm::IRBuilder<>& builder;
  };

} // namespace codegen

#endif //INCLUDE_CODEGEN_OPERATION_BUILDER_H_ 
