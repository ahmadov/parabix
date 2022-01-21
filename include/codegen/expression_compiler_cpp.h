#ifndef INCLUDE_CODEGEN_EXPRESSION_COMPILER_CPP_H_
#define INCLUDE_CODEGEN_EXPRESSION_COMPILER_CPP_H_

#include "codegen/ast.h"
#include "codegen/jit.h"
#include <unordered_map>
#include <sstream>

namespace codegen {
  
  class ExpressionCompilerCpp {
    public:
      explicit ExpressionCompilerCpp() = default;

      std::string compile(BitwiseExpression& expression);

      uint64_t execute(std::array<uint64_t, 8>& basis, BitwiseExpression* expression);

    private:
      std::stringstream output;
      uint32_t variable_counter;
      std::unordered_map<std::string, std::string> variables;

      std::string compileExpression(BitwiseExpression* expression);

      std::string getVariable(std::string& generated);

      void addAssignment(std::string& variable, std::string& generated);

      void reset() noexcept {
       variable_counter = 0;
       variables.clear();
       output.clear();
      }
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_EXPRESSION_COMPILER_CPP_H_

