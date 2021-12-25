#ifndef INCLUDE_CODEGEN_AST_H_
#define INCLUDE_CODEGEN_AST_H_

#include <cstdint>
#include <string>
#include <memory>

const uint8_t ENCODING_BITS = 8;

namespace codegen {

  struct BitwiseExpression {
    /// Destructor
    virtual ~BitwiseExpression() = default;

    virtual std::string as_string() = 0;
  };

  struct Bit: public BitwiseExpression {
    /// The constant value.
    uint8_t bit;

    /// Constructor.
    explicit Bit(uint8_t bit)
      : bit(bit) {}

    std::string as_string() override {
      return "Bit(" + std::to_string((ENCODING_BITS - 1) - bit) + ")";
    }
  };

  struct BinaryExpression: public BitwiseExpression {
    /// The left child.
    std::unique_ptr<BitwiseExpression> left;
    /// The right child.
    std::unique_ptr<BitwiseExpression> right;

    /// Constructor.
    BinaryExpression(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right)
      : left(std::move(left)), right(std::move(right)) {
    }

    std::string as_string() override {
      return "Binary(" + left->as_string() + ", " + right->as_string() + ")";
    }
  };

  struct AndExpression: public BinaryExpression {
    /// Constructor
    AndExpression(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right)
      : BinaryExpression(std::move(left), std::move(right)) {}

    std::string as_string() override {
      return "And(" + left->as_string() + ", " + right->as_string() + ")";
    }
  };

  struct OrExpression: public BinaryExpression {
    /// Constructor
    OrExpression(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right)
      : BinaryExpression(std::move(left), std::move(right)) {}

    std::string as_string() override {
      return "Or(" + left->as_string() + ", " + right->as_string() + ")";
    }
  };

  struct XorExpression: public BinaryExpression {
    /// Constructor
    XorExpression(std::unique_ptr<BitwiseExpression> left, std::unique_ptr<BitwiseExpression> right)
      : BinaryExpression(std::move(left), std::move(right)) {}

    std::string as_string() override {
      return "Xor(" + left->as_string() + ", " + right->as_string() + ")";
    }
  };

  struct NotExpression: public BitwiseExpression {
    /// The child.
    std::unique_ptr<BitwiseExpression> child;

    /// Constructor
    explicit NotExpression(std::unique_ptr<BitwiseExpression> child)
      : child(std::move(child)) {}

    std::string as_string() override {
      return "Not(" + child->as_string() + ")";
    }
  };

} // namespace codegen

#endif  // INCLUDE_CODEGEN_AST_H_
