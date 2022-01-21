#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <chrono> // NOLINT
#include <immintrin.h>
#include "PerfEvent.hpp"
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/ast.h"
#include "codegen/expression_compiler_cpp.h"

void print_help(const char* name) {
  std::cerr << "usage: " << name << " [/path/to/file] [regex]" << std::endl;
}

#ifndef PRINT
#define PRINT false 
#endif

void print_table(std::vector<uint64_t>& stream, std::string_view name) {
  for (auto& elem : stream) {
    std::cout << std::setw(4) << std::left << name;
    for (auto j = 0; j < 63; ++j) {
      std::cout << ((elem >> j) & 1 ? "1" : ".");
    }
    std::cout << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    print_help(argv[0]);
    exit(0);
  }

  std::ifstream t(argv[1]);
  std::stringstream buffer;
  buffer << t.rdbuf();

  auto input = buffer.str();
  auto pattern = argv[2];

  std::cout << "input size: " << input.size() << std::endl;

  auto tick = std::chrono::high_resolution_clock::now();

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

  PerfEvent e;
  e.startCounters();

  std::vector<std::unique_ptr<codegen::BitwiseExpression>> expressions(cc_size);
  for (auto i = 0; i < cc_size; ++i) {
    expressions[i] = cc_compiler.compile(cc_list[i]);
  }

  codegen::ExpressionCompilerCpp expr_compiler_cpp;
  for (size_t i = 0, block = 0; i < input_size; i += block_size, ++block) {
#if PRINT
    std::cout << "processing block " << block << std::endl;
#endif
    auto to = std::min(block_size, input_size - i);

    std::array<uint64_t, 8> basis = {0};
    // TODO(me): use transpose matrix
    for (auto j = 0; j < to; ++j) {
      for (size_t k = 0; k < 8; ++k) {
        if ((input[i + j] >> k) & 1) {
          basis[k] |= (1ULL << j);
        }
      }
    }

#if PRINT
    print_table(basis, "B");
#endif

    std::vector<uint64_t> cc(cc_size);
    for (auto i = 0; i < cc_size; ++i) {
      cc[i] = expr_compiler_cpp.execute(basis, expressions[i].get());
    }

#if PRINT
    print_table(cc, "CC");
#endif

    auto markers_size = cc_size + 1;
    std::vector<uint64_t> marker(markers_size);
    marker[0] = cc[0];
    for (size_t i = 0; i < cc_size; ++i) {
      if (cc_list[i].isStar()) {
        auto M = marker[i];
        M &= cc[i];
        M += cc[i] + carry[i];
        carry[i] = (M >> to) & 1;
        M &= ~(1ULL << block_size);
        M ^= cc[i];
        M |= marker[i];
        marker[i + 1] = M;
      } else {
        auto M = marker[i];
        M &= cc[i];
        bool next_carry = (M >> (block_size - 1)) & 1;
        M <<= 1;
        M &= ~(1ULL << block_size);
        M |= carry[i];
        marker[i + 1] = M;
        carry[i] = next_carry;
      }
    }

#if PRINT
    print_table(marker, "M");
#endif

    matched += _mm_popcnt_u64(marker.back());
  }

  e.stopCounters();
  e.printReport(std::cout, input_size); // use n as scale factor

  std::cout << "matched = " << matched << std::endl;

  auto tock = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time = tock - tick;
  std::cout << "elapsed time = " << elapsed_time.count() << " second" << std::endl;
  
  return 0;
}
