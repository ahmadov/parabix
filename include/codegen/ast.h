#ifndef INCLUDE_CODEGEN_AST_H_
#define INCLUDE_CODEGEN_AST_H_

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <cstdint>
#include <string>
#include <memory>

const uint8_t ENCODING_BITS = 8;

namespace codegen {

  struct BitwiseExpression {
    enum class Type {
      Bit,
      True,
      False,
      And,
      Or,
      Not,
      Selection,
    };

    /// The bitwise expression type.
    Type type;

    /// Constructor.
    explicit BitwiseExpression(Type type): type(type) {}

    /// Destructor
    virtual ~BitwiseExpression() = default;

    virtual std::string as_string() = 0;

    virtual llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) = 0;

    /// Get the expression type.
    [[nodiscard]] constexpr Type getType() const { return type; }
  };

  struct True: public BitwiseExpression {
    /// Constructor.
    explicit True(): BitwiseExpression(Type::True) {}

    std::string as_string() override {
      return "True()";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      return nullptr;
    }
  };

  struct False: public BitwiseExpression {
    /// Constructor.
    explicit False(): BitwiseExpression(Type::False) {}

    std::string as_string() override {
      return "False()";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      return nullptr;
    }
  };

  struct Bit: public BitwiseExpression {
    /// The constant value.
    uint8_t bit;

    /// Constructor.
    explicit Bit(uint8_t bit)
      : BitwiseExpression(Type::Bit)
      , bit(bit) {}

    std::string as_string() override {
      return "Bit(" + std::to_string((ENCODING_BITS - 1) - bit) + ")";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      if (bit == 0) {
        return builder.CreateAnd(arg, 1);
      }
      return builder.CreateAnd(builder.CreateLShr(arg, bit), 1);
    }
  };

  struct SelectionExpression: public BitwiseExpression {
    /// The if condition expression
    std::unique_ptr<BitwiseExpression> if_expr;
    /// The true branch expression.
    std::unique_ptr<BitwiseExpression> true_expr;
    /// The false branch expression.
    std::unique_ptr<BitwiseExpression> false_expr;

    /// Constructor.
    SelectionExpression(
      std::unique_ptr<BitwiseExpression> if_expr,
      std::unique_ptr<BitwiseExpression> true_expr,
      std::unique_ptr<BitwiseExpression> false_expr
    )
      : BitwiseExpression(Type::Selection)
      , if_expr(std::move(if_expr))
      , true_expr(std::move(true_expr))
      , false_expr(std::move(false_expr)) {}

    std::string as_string() override {
      return "Selection(" + if_expr->as_string() + " ? " + true_expr->as_string() + " : " + false_expr->as_string() + ")";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      auto if_expr_value = if_expr->codegen(builder, arg);
      return builder.CreateOr(
        builder.CreateAnd(if_expr_value, true_expr->codegen(builder, arg)),
        builder.CreateAnd(builder.CreateXor(if_expr_value, -1), false_expr->codegen(builder, arg))
      );
    }
  };

  struct BinaryExpression: public BitwiseExpression {
    /// The left child.
    std::unique_ptr<BitwiseExpression> left;
    /// The right child.
    std::unique_ptr<BitwiseExpression> right;

    /// Constructor.
    BinaryExpression(Type type, std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right)
      : BitwiseExpression(type)
      , left(std::move(left))
      , right(std::move(right)) {}

    std::string as_string() override {
      return "Binary(" + left->as_string() + ", " + right->as_string() + ")";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      return nullptr;
    }
  };

  struct AndExpression: public BinaryExpression {
    /// Constructor
    AndExpression(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right)
      : BinaryExpression(Type::And, std::move(left), std::move(right)) {}

    std::string as_string() override {
      return "And(" + left->as_string() + ", " + right->as_string() + ")";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      return builder.CreateAnd(left->codegen(builder, arg), right->codegen(builder, arg));
    }
  };

  struct OrExpression: public BinaryExpression {
    /// Constructor
    OrExpression(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right)
      : BinaryExpression(Type::Or, std::move(left), std::move(right)) {}

    std::string as_string() override {
      return "Or(" + left->as_string() + ", " + right->as_string() + ")";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      return builder.CreateOr(left->codegen(builder, arg), right->codegen(builder, arg));
    }
  };

  struct NotExpression: public BitwiseExpression {
    /// The child.
    std::unique_ptr<BitwiseExpression> child;

    /// Constructor
    explicit NotExpression(std::unique_ptr<BitwiseExpression> child)
      : BitwiseExpression(Type::Not)
      , child(std::move(child)) {}

    std::string as_string() override {
      return "Not(" + child->as_string() + ")";
    }

    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Value* arg) override {
      return builder.CreateXor(child->codegen(builder, arg), -1);
    }
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_AST_H_
