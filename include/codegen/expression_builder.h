#ifndef INCLUDE_CODEGEN_EXPRESSION_BUILDER_H_
#define INCLUDE_CODEGEN_EXPRESSION_BUILDER_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/ErrorHandling.h>
#include "codegen/ast.h"

namespace codegen {

  class ExpressionBuilder {
    public:

      explicit ExpressionBuilder(llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& arguments)
        : builder(builder)
        , arguments(arguments) {}

      llvm::Value* codegen(BitwiseExpression* expression);

      llvm::Value* createBit(Bit* expression, llvm::Value* argument);

      llvm::Value* createAnd(BinaryExpression* expression);

      llvm::Value* createOr(BinaryExpression* expression);

      llvm::Value* createNeg(BitwiseExpression* expression);

      llvm::Value* createSelection(SelectionExpression* expression);

    private:
      llvm::IRBuilder<>& builder;
      std::vector<llvm::Value*> arguments;
      std::unordered_map<std::string, llvm::Value*> cache;
  };

} // namespace codegen

#endif //INCLUDE_CODEGEN_EXPRESSION_BUILDER_H_ 
