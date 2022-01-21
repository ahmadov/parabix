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

void transpose_sse(char *inp, uint8_t *out, int n_rows) {
  int n_cols = 8;
#   define INP(x,y) inp[(x)*n_cols/8 + (y)/8]
#   define OUT(x,y) out[(y)*n_rows/8 + (x)/8]
  int rr, cc, i, h;
  union { __m128i x; uint8_t b[16]; } tmp{};

  // Do the main body in 16x8 blocks:
  for (rr = 0; rr <= n_rows - 16; rr += 16) {
    for (cc = 0; cc < n_cols; cc += 8) {
      for (i = 0; i < 16; ++i) {
        tmp.b[i] = INP(rr + i, cc);
      }
      for (i = 0; i < 8; ++i, tmp.x = _mm_slli_epi64(tmp.x, 1)) {
        *reinterpret_cast<uint16_t*>(&OUT(rr,cc+i))= _mm_movemask_epi8(tmp.x);
      }
    }
  }
  if (rr == n_rows) return;

  // The remainder is a block of 8x(16n+8) bits (n may be 0).
  //  Do a PAIR of 8x8 blocks in each step:
  for (cc = 0; cc <= n_cols - 16; cc += 16) {
    for (i = 0; i < 8; ++i) {
      tmp.b[i] = h = *reinterpret_cast<uint16_t const*>(&INP(rr + i, cc));
      tmp.b[i + 8] = h >> 8;
    }
    for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1)) {
      OUT(rr, cc + i) = h = _mm_movemask_epi8(tmp.x);
      OUT(rr, cc + i + 8) = h >> 8;
    }
  }
  if (cc == n_cols) return;

  //  Do the remaining 8x8 block:
  for (i = 0; i < 8; ++i) {
    tmp.b[i] = INP(rr + i, cc);
  } 
  for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1)) {
    OUT(rr, cc + i) = _mm_movemask_epi8(tmp.x);
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

    std::array<uint8_t, 64> output;

    transpose_sse(input.data() + i, output.data(), 64);

    std::array<uint64_t, 8> basis = {0};
    for (auto i = 7; i >= 0; --i) {
       auto offset = i * 8;
       for (auto j = 7; j >= 0; --j) {
        auto current = static_cast<uint64_t>(output[offset + j]);
        basis[7 - i] |= (current << (j * 8));
       }
       basis[7 - i] &= ~(1ULL << block_size);
    }

#if PRINT
    print_basis_table(basis, "B");
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
