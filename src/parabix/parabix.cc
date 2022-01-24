#include <iomanip>
#include <iostream>
#include <popcntintrin.h>

#include "parabix/parabix.h"
#include "parabix/bit.h"
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/expression_compiler_cpp.h"
#include "codegen/parabix_compiler.h"

#ifndef PRINT
  #define PRINT false
#endif

#if PRINT
void print_basis_table(std::array<uint64_t, 8>& arr, std::string_view name) {
  for (auto& elem : arr) {
    std::cout << std::setw(4) << std::left << name;
    for (auto j = 0; j < 63; ++j) {
      std::cout << ((elem >> j) & 1 ? "1" : ".");
    }
    std::cout << std::endl;
  }
}

void print_table(std::vector<uint64_t>& stream, std::string_view name) {
  for (auto& elem : stream) {
    std::cout << std::setw(4) << std::left << name;
    for (auto j = 0; j < 63; ++j) {
      std::cout << ((elem >> j) & 1 ? "1" : ".");
    }
    std::cout << std::endl;
  }
}
#endif

uint64_t parabix::parabix_cpp(std::string& input, const char* pattern) {
  parser::ReParser parser;
  codegen::CCCompiler cc_compiler;

  auto cc_list = parser.parse(pattern);
  auto input_size = input.length();
  auto cc_size = cc_list.size();

  size_t block_size = 63;
  uint64_t matched = 0;
  std::vector<bool> carry(cc_size, false);

#if PRINT
  std::cout << "    " << input << std::endl;
#endif

  std::vector<std::unique_ptr<codegen::BitwiseExpression>> expressions(cc_size);
  for (auto i = 0; i < cc_size; ++i) {
    expressions[i] = cc_compiler.compile(cc_list[i]);
  }

  auto markers_size = cc_size + 1;
  std::vector<uint64_t> cc(cc_size);
  std::vector<uint64_t> marker(markers_size);
  codegen::ExpressionCompilerCpp expr_compiler_cpp;
  std::array<uint8_t, 64> output;
  std::array<uint64_t, 8> basis;
  for (size_t i = 0, block = 0; i < input_size; i += block_size, ++block) {
#if PRINT
    std::cout << "processing block " << block << std::endl;
#endif
    transpose_sse(input.data() + i, output.data());
    
    for (auto i = 0, j = 7; i < 8; ++i, --j) {
       basis[i] = *reinterpret_cast<uint64_t*>(&output[static_cast<unsigned>(j * 8)]);
       basis[i] &= ~(1ULL << block_size);
    }

#if PRINT
    print_basis_table(basis, "B");
#endif

    for (auto i = 0; i < cc_size; ++i) {
      cc[i] = expr_compiler_cpp.execute(basis, expressions[i].get());
    }

#if PRINT
    print_table(cc, "CC");
#endif

    marker[0] = cc[0];
    for (size_t i = 0; i < cc_size; ++i) {
      if (cc_list[i].isStar()) {
        auto M = marker[i];
        M &= cc[i];
        M += cc[i] + carry[i];
        carry[i] = (M >> block_size) & 1;
        M &= ~(1ULL << block_size);
        M ^= cc[i];
        M |= marker[i];
        marker[i + 1] = M;
      } else {
        auto M = marker[i];
        M &= cc[i];
        M <<= 1;
        M |= carry[i];
        carry[i] = (M >> block_size) & 1;
        M &= ~(1ULL << block_size);
        marker[i + 1] = M;
      }
    }

#if PRINT
    print_table(marker, "M");
#endif

    matched += _mm_popcnt_u64(marker.back());
  }

  return matched;
}

uint64_t parabix::parabix_llvm(llvm::orc::ThreadSafeContext& context, std::string& input, const char* pattern) {
    parser::ReParser parser;
  codegen::CCCompiler cc_compiler;

  auto cc_list = parser.parse(pattern);
  auto input_size = input.length();
  auto cc_size = cc_list.size();

  size_t block_size = 63;
  uint64_t matched = 0;
  std::vector<bool> carry(cc_size, false);

#if PRINT
  std::cout << "    " << input << std::endl;
#endif

  std::vector<std::unique_ptr<codegen::BitwiseExpression>> expressions(cc_size);
  for (auto i = 0; i < cc_size; ++i) {
    expressions[i] = cc_compiler.compile(cc_list[i]);
  }

  codegen::ParabixCompiler compiler(context);
  compiler.compile(expressions, false);

  auto markers_size = cc_size + 1;
  std::vector<uint64_t> cc(cc_size);
  std::vector<uint64_t> marker(markers_size);
  std::array<uint8_t, 64> output;
  std::array<uint64_t, 8> basis;
  for (size_t i = 0, block = 0; i < input_size; i += block_size, ++block) {
#if PRINT
    std::cout << "processing block " << block << std::endl;
#endif
    transpose_sse(input.data() + i, output.data());
    
    for (auto i = 0, j = 7; i < 8; ++i, --j) {
       basis[i] = *reinterpret_cast<uint64_t*>(&output[static_cast<unsigned>(j * 8)]);
       basis[i] &= ~(1ULL << block_size);
    }

#if PRINT
    print_basis_table(basis, "B");
#endif

    for (auto i = 0; i < cc_size; ++i) {
      compiler.runMatch(basis.data(), cc.data());
    }

#if PRINT
    print_table(cc, "CC");
#endif

    marker[0] = cc[0];
    for (size_t i = 0; i < cc_size; ++i) {
      if (cc_list[i].isStar()) {
        auto result = compiler.runMatchStar(marker[i], cc[i], carry[i]);
        carry[i] = result.carry;
        marker[i + 1] = result.marker;
      } else {
        auto result = compiler.runAdvance(marker[i], cc[i], carry[i]);
        carry[i] = result.carry;
        marker[i + 1] = result.marker;
      }
    }

#if PRINT
    print_table(marker, "M");
#endif

    matched += _mm_popcnt_u64(marker.back());
  }

  return matched;
}
