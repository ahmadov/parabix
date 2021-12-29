#include "codegen/expression_builder.h"

using ExpressionBuilder = codegen::ExpressionBuilder;
using BitwiseExpression = codegen::BitwiseExpression;
using Bit = codegen::Bit;
using BinaryExpression = codegen::BinaryExpression;
using SelectionExpression = codegen::SelectionExpression;
using ExprType = codegen::BitwiseExpression::Type;
using ValueType = llvm::Value*;

ValueType ExpressionBuilder::codegen(BitwiseExpression* expression) {
  const auto& as_string = expression->as_string();
  if (cache.count(as_string)) {
    return cache[as_string];
  }
  switch(expression->getType()) {
    case ExprType::Bit:
      return cache[as_string] = createBit(dynamic_cast<Bit*>(expression), arguments[0]);
    case ExprType::And:
      return cache[as_string] = createAnd(dynamic_cast<BinaryExpression*>(expression));
    case ExprType::Or:
      return cache[as_string] = createOr(dynamic_cast<BinaryExpression*>(expression));
    case ExprType::Selection:
      return cache[as_string] = createSelection(dynamic_cast<SelectionExpression*>(expression));
    case ExprType::Not:
      return cache[as_string] = createNeg(dynamic_cast<NotExpression*>(expression)->child.get());
    case ExprType::True:
    case ExprType::False:
      return nullptr;
  }
  llvm_unreachable("all types should be handled properly");
}

ValueType ExpressionBuilder::createBit(Bit* expression, ValueType argument) {
  if (expression->bit == 0) {
    return builder.CreateAnd(argument, 1);
  }
  return builder.CreateAnd(builder.CreateLShr(argument, expression->bit), 1);
}

ValueType ExpressionBuilder::createAnd(BinaryExpression* expression) {
  const auto& as_string = expression->as_string();
  if (cache.count(as_string)) {
    return cache[as_string];
  }
  auto* left = codegen(expression->left.get());
  auto* right = codegen(expression->right.get());
  auto* value = builder.CreateAnd(left, right);
  return cache[as_string] = value;
}

ValueType ExpressionBuilder::createOr(BinaryExpression* expression) {
  const auto& as_string = expression->as_string();
  if (cache.count(as_string)) {
    return cache[as_string];
  }
  auto* left = codegen(expression->left.get());
  auto* right = codegen(expression->right.get());
  auto* value = builder.CreateOr(left, right);
  return cache[as_string] = value;
}

ValueType ExpressionBuilder::createSelection(SelectionExpression* expression) {
  const auto& as_string = expression->as_string();
  if (cache.count(as_string)) {
    return cache[as_string];
  }
  auto* if_expr_value = codegen(expression->if_expr.get());
  auto* value = builder.CreateOr(
    builder.CreateAnd(if_expr_value, codegen(expression->true_expr.get())),
    builder.CreateAnd(builder.CreateXor(if_expr_value, -1), codegen(expression->false_expr.get()))
  );
  return cache[as_string] = value;
}

ValueType ExpressionBuilder::createNeg(BitwiseExpression* expression) {
  const auto& as_string = expression->as_string();
  if (cache.count(as_string)) {
    return cache[as_string];
  }
  auto* value = builder.CreateXor(codegen(expression), -1);
  return cache[as_string] = value;
}
