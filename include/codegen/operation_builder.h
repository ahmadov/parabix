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

      explicit OperationBuilder(
        llvm::IRBuilder<>& builder,
        const std::vector<parser::CC>& cc_list,
        llvm::Value* cc_bit_stream,
        llvm::Value* marker_bit_stream,
        llvm::Value* carry_bit_stream
      )
        : builder(builder)
        , cc_list(cc_list)
        , cc_bit_stream(cc_bit_stream) 
        , marker_bit_stream(marker_bit_stream) 
        , carry_bit_stream(carry_bit_stream) {
        initializeGEPs();
      }

      void codegen();

    private:
      void initializeGEPs();
    
      void buildMatchStar(size_t index);

      void buildAdvance(size_t index);

      llvm::IRBuilder<>& builder;
      std::vector<parser::CC> cc_list;
      llvm::Value* cc_bit_stream;
      llvm::Value* marker_bit_stream;
      llvm::Value* carry_bit_stream;

      std::vector<llvm::Value*> marker_ptr;
      std::vector<llvm::Value*> cc_ptr;
      std::vector<llvm::Value*> carry_ptr;
  };

} // namespace codegen

#endif //INCLUDE_CODEGEN_OPERATION_BUILDER_H_ 
