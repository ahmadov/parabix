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

#ifndef PRINT
#define PRINT false 
#endif

void print_help(const char* name) {
  std::cerr << "usage: " << name << " [/path/to/file] [regex]" << std::endl;
}

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

void transpose_sse(char *inp, uint8_t *out) {
# define OUT(x,y) out[(y)*8 + (x)/8]
  int rr, i, h;
  union { __m128i x; uint8_t b[16]; } tmp{};

  // Do the main body in 16x8 blocks:
  for (rr = 0; rr <= 48; rr += 16) {
    for (i = 0; i < 16; ++i) {
      tmp.b[i] = inp[rr + i];
    }
    for (i = 0; i < 8; ++i, tmp.x = _mm_slli_epi64(tmp.x, 1)) {
      *reinterpret_cast<uint16_t*>(&OUT(rr,i))= _mm_movemask_epi8(tmp.x);
    }
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
    auto to = std::min(block_size, input_size - i);

    transpose_sse(input.data() + i, output.data());
    
    for (auto i = 0; i < 8; ++i) {
       basis[i] = 0;
       auto offset = (7 - i) * 8;
       for (auto j = 0; j < 8; ++j) {
        auto current = static_cast<uint64_t>(output[offset + j]);
        basis[i] |= (current << (j * 8));
       }
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
        carry[i] = (M >> to) & 1;
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

  e.stopCounters();
  e.printReport(std::cout, input_size); // use n as scale factor

  std::cout << "matched = " << matched << std::endl;

  auto tock = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time = tock - tick;
  std::cout << "elapsed time = " << elapsed_time.count() << " second" << std::endl;
  
  return 0;
}
