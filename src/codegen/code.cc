#include "codegen/code.h"

using CompiledCode = codegen::CompiledCode;
using ExprType = codegen::BitwiseExpression::Type;

std::unique_ptr<CompiledCode> CompiledCode::compileCCFrom(BitwiseExpression* expression) {
   auto* code = new CompiledCode();
   code->output << "bool match(uint8_t value) {\n";
   for (auto i = 0; i < ENCODING_BITS; ++i) {
      code->output << "  auto bit_" + std::to_string(i) + " = (value >> " + std::to_string((ENCODING_BITS - 1) - i) << ") & 1;\n";
   }
   auto result = code->generateCC(expression);
   code->output << "  return " << result << ";\n";
   code->output << "}";
   return std::unique_ptr<CompiledCode>(code);
}

std::string CompiledCode::generateCC(BitwiseExpression* expression) {
   switch (expression->getType()) {
      case ExprType::True:
         return "-1";
      case ExprType::False:
         return "0";
      case ExprType::Bit: {
         auto expr_ptr = dynamic_cast<Bit*>(expression);
         auto variable = "bit_" + std::to_string((ENCODING_BITS - 1) - expr_ptr->bit);
         variables.emplace(variable, variable);
         return variable;
      }
      case ExprType::Not: {
         auto expr_ptr = dynamic_cast<NotExpression*>(expression);
         auto generated = generateCC(expr_ptr->child.get());
         return "(~" + getVariableCC(generated) + ")";
      }
      case ExprType::And: {
         auto expr_ptr = dynamic_cast<BinaryExpression*>(expression);
         if (expr_ptr->left->getType() == ExprType::Not) {
            auto left_expr = generateCC(dynamic_cast<NotExpression*>(expr_ptr->left.get())->child.get());
            auto right_expr = generateCC(expr_ptr->right.get());
            // %s & ~%s
            return "(" + getVariableCC(right_expr) + " &~ " + getVariableCC(left_expr) + ")";
         } else if (expr_ptr->right->getType() == ExprType::Not) {
            auto left_expr = generateCC(expr_ptr->left.get());
            auto right_expr = generateCC(dynamic_cast<NotExpression*>(expr_ptr->right.get())->child.get());
            // %s & ~%s
            return "(" + getVariableCC(left_expr) + " &~ " + getVariableCC(right_expr) + ")";
         } else {
            auto left_expr = generateCC(expr_ptr->left.get());
            auto right_expr = generateCC(expr_ptr->right.get());
            // %s & %s
            return "(" + getVariableCC(left_expr) + " & " + getVariableCC(right_expr) + ")";
         }
      }
      case ExprType::Or: {
         auto expr_ptr = dynamic_cast<BinaryExpression*>(expression);
         auto left_expr = generateCC(expr_ptr->left.get());
         auto right_expr = generateCC(expr_ptr->right.get());
         // %s & %s
         return "(" + getVariableCC(left_expr) + " | " + getVariableCC(right_expr) + ")";
      }
      case ExprType::Selection: {
         auto expr_ptr = dynamic_cast<SelectionExpression*>(expression);
         auto if_expr = generateCC(expr_ptr->if_expr.get());
         auto left_expr = generateCC(expr_ptr->true_expr.get());
         auto right_expr = generateCC(expr_ptr->false_expr.get());
         auto left = "(" + getVariableCC(if_expr) + " & " + getVariableCC(left_expr) + ")";
         auto right = "(~(" + getVariableCC(if_expr) + ") & " + getVariableCC(right_expr) + ")";
         // (%s & true) | (~(%s) & false)
         return left + " | " + right;
      }
   }
   throw std::runtime_error{"unknown expression type"};
}

std::string CompiledCode::getVariableCC(std::string& generated) {
   if (variables.count(generated)) {
      return variables[generated];
   }
   auto variable = "tmp_" + std::to_string(++variable_counter);
   addAssignmentCC(variable, generated);
   return variable;
}

void CompiledCode::addAssignmentCC(std::string& variable, std::string& generated) {
   std::string assigned;
   if (variables.count(generated)) {
      assigned = variables[generated];
      if (assigned == variable) {
         return;
      }
   } else {
      variables[generated] = variable;
      assigned = generated;
   }
   output << "  auto " << variable << " = " << assigned << ";\n";
}
