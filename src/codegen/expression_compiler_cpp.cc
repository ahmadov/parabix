#include "codegen/expression_compiler_cpp.h"

using ExpressionCompilerCpp = codegen::ExpressionCompilerCpp;
using ExprType = codegen::BitwiseExpression::Type;

std::string ExpressionCompilerCpp::compile(BitwiseExpression& expression) {
   reset();
   output << "bool match(uint8_t value) {\n";
   for (auto i = 0; i < ENCODING_BITS - 1; ++i) {
      output << "  auto bit_" + std::to_string(i) + " = (value >> " + std::to_string((ENCODING_BITS - 1) - i) << ") & 1;\n";
   }
   output << "  auto bit_" + std::to_string(ENCODING_BITS - 1) + " = value & 1;\n"; // `>> 0` does nothing.
   auto result = compileExpression(&expression);
   output << "  return " << result << ";\n";
   output << "}";
   return output.str();
}

std::string ExpressionCompilerCpp::compileExpression(BitwiseExpression* expression) {
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
         auto generated = compileExpression(expr_ptr->child.get());
         return "(~" + getVariable(generated) + ")";
      }
      case ExprType::And: {
         auto expr_ptr = dynamic_cast<BinaryExpression*>(expression);
         if (expr_ptr->left->getType() == ExprType::Not) {
            auto left_expr = compileExpression(dynamic_cast<NotExpression*>(expr_ptr->left.get())->child.get());
            auto right_expr = compileExpression(expr_ptr->right.get());
            // %s & ~%s
            return "(" + getVariable(right_expr) + " &~ " + getVariable(left_expr) + ")";
         } else if (expr_ptr->right->getType() == ExprType::Not) {
            auto left_expr = compileExpression(expr_ptr->left.get());
            auto right_expr = compileExpression(dynamic_cast<NotExpression*>(expr_ptr->right.get())->child.get());
            // %s & ~%s
            return "(" + getVariable(left_expr) + " &~ " + getVariable(right_expr) + ")";
         } else {
            auto left_expr = compileExpression(expr_ptr->left.get());
            auto right_expr = compileExpression(expr_ptr->right.get());
            // %s & %s
            return "(" + getVariable(left_expr) + " & " + getVariable(right_expr) + ")";
         }
      }
      case ExprType::Or: {
         auto expr_ptr = dynamic_cast<BinaryExpression*>(expression);
         auto left_expr = compileExpression(expr_ptr->left.get());
         auto right_expr = compileExpression(expr_ptr->right.get());
         // %s & %s
         return "(" + getVariable(left_expr) + " | " + getVariable(right_expr) + ")";
      }
      case ExprType::Selection: {
         auto expr_ptr = dynamic_cast<SelectionExpression*>(expression);
         auto if_expr = compileExpression(expr_ptr->if_expr.get());
         auto left_expr = compileExpression(expr_ptr->true_expr.get());
         auto right_expr = compileExpression(expr_ptr->false_expr.get());
         auto left = "(" + getVariable(if_expr) + " & " + getVariable(left_expr) + ")";
         auto right = "(~(" + getVariable(if_expr) + ") & " + getVariable(right_expr) + ")";
         // (%s & true) | (~(%s) & false)
         return left + " | " + right;
      }
   }
   throw std::runtime_error{"unknown expression type"};
}

std::string ExpressionCompilerCpp::getVariable(std::string& generated) {
   if (variables.count(generated)) {
      return variables[generated];
   }
   auto variable = "tmp_" + std::to_string(variable_counter++);
   addAssignment(variable, generated);
   return variable;
}

void ExpressionCompilerCpp::addAssignment(std::string& variable, std::string& generated) {
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
