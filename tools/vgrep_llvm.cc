#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <chrono> // NOLINT
#include <immintrin.h>
#include "PerfEvent.hpp"
#include "codegen/operation_compiler.h"
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/ast.h"
#include "codegen/expression_compiler_llvm.h"

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

// simplified version of https://mischasan.wordpress.com/2011/10/03/the-full-sse2-bit-matrix-transpose-routine/ 
void transpose_sse(char *inp, uint8_t *out) {
  int rr, i, h;
  union { __m128i x; uint8_t b[16]; } tmp{};

  // Do the main body in 16x8 blocks:
  for (rr = 0; rr < 4; ++rr) {
    for (i = 0; i < 16; ++i) {
      tmp.b[i] = inp[rr * 16 + i];
    }
    for (i = 0; i < 8; ++i, tmp.x = _mm_slli_epi64(tmp.x, 1)) {
      *reinterpret_cast<uint16_t*>(&out[(rr * 2) + (i * 8)]) = _mm_movemask_epi8(tmp.x);
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

  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  llvm::orc::ThreadSafeContext context(std::make_unique<llvm::LLVMContext>());
  std::vector<std::unique_ptr<codegen::BitwiseExpression>> expressions(cc_size);
  for (auto i = 0; i < cc_size; ++i) {
    expressions[i] = cc_compiler.compile(cc_list[i]);
  }

  codegen::ExpressionCompiler expr_compiler(context);
  expr_compiler.compile(expressions, false);

  codegen::OperationCompiler operation_compiler(context);
  operation_compiler.initialize(false);

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
      expr_compiler.run(basis.data(), cc.data());
    }

#if PRINT
    print_table(cc, "CC");
#endif

    marker[0] = cc[0];
    for (size_t i = 0; i < cc_size; ++i) {
      if (cc_list[i].isStar()) {
        auto result = operation_compiler.runMatchStar(marker[i], cc[i], carry[i]);
        carry[i] = result.carry;
        marker[i + 1] = result.marker;
      } else {
        auto result = operation_compiler.runAdvance(marker[i], cc[i], carry[i]);
        carry[i] = result.carry;
        marker[i + 1] = result.marker;
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
