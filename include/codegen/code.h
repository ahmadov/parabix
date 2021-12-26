#ifndef INCLUDE_CODEGEN_CODE_H_
#define INCLUDE_CODEGEN_CODE_H_

#include "codegen/ast.h"
#include <unordered_map>
#include <sstream>
#include <memory>

namespace codegen {
  
  class CompiledCode {
    public:
      static std::unique_ptr<CompiledCode> compileCCFrom(BitwiseExpression* expression);

      [[nodiscard]] std::string getOutputCC() const { return output.str(); }

    private:
      std::stringstream output;
      uint32_t variable_counter;
      std::unordered_map<std::string, std::string> variables;

      explicit CompiledCode()
        : variable_counter(0) {}

      std::string generateCC(BitwiseExpression* expression);

      std::string getVariableCC(std::string& generated);

      void addAssignmentCC(std::string& variable, std::string& generated);
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_CODE_H_

