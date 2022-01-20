#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <chrono> // NOLINT
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/ast.h"
#include "codegen/expression_compiler_cpp.h"
#include "codegen/expression_compiler_llvm.h"
#include "stream/bit_stream.h"
#include "operations/marker.h"

void print_help(const char* name) {
  std::cerr << "usage: " << name << " [/path/to/file] [regex]" << std::endl;
}

void print_red(std::string_view output) {
  std::cout << "\e[31m" << output.data() << "\e[39m";
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

#if LLVM
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  llvm::orc::ThreadSafeContext context(std::make_unique<llvm::LLVMContext>());
  std::vector<std::unique_ptr<codegen::BitwiseExpression>> expressions;
  for (auto& cc : cc_list) {
    auto expression = cc_compiler.compile(cc);
#if 0 // use DEBUG flag
    std::cout << std::setw(10) << std::left << cc << " => " << expression->as_string() << std::endl;
#endif

#if 0 // use DEBUG flag
    codegen::ExpressionCompilerCpp expr_compiler_cpp;
    std::cout << expr_compiler_cpp.compile(*expression) << std::endl;
#endif

    expressions.push_back(std::move(expression));
  }

  codegen::ExpressionCompiler expr_compiler(context);
  expr_compiler.compile(expressions, false);

  // CC bit streams
  std::vector<stream::BitStream> cc_bit_streams(cc_size, stream::BitStream(input_size));
  for (size_t i = 0; i < input_size; ++i) {
    std::vector<uint8_t> match_result(cc_size);
    expr_compiler.run(input[i], reinterpret_cast<uint8_t*>(match_result.data()));
    for (size_t j = 0; j < cc_size; ++j) {
      cc_bit_streams[j].set(i, match_result[j]);
    }
  }
#else
  std::vector<stream::BitStream> cc_bit_streams(cc_size, stream::BitStream(input_size));
  for (size_t i = 0; i < input_size; ++i) {
    for (size_t j = 0; j < cc_size; ++j) {
      cc_bit_streams[j].set(i, cc_list[j].match(input[i]));
    }
  }

#endif

  auto cc_width = 15;
  int width = static_cast<int>(input_size + cc_width);
  std::cout << std::setw(width) << std::right << input << std::endl;
  for (size_t i = 0; i < cc_size; ++i) {
    auto& cc = cc_list[i];
    std::cout << std::setw(cc_width) << std::left << cc;
    std::cout << cc_bit_streams[i] << std::endl;
  }
  
  // Marker bit streams
  auto markers_size = cc_size + 1;
  std::vector<stream::BitStream> markers(markers_size, stream::BitStream(input_size));
  markers[0] = cc_bit_streams[0];
  for (size_t i = 0; i < markers_size - 1; ++i) {
    if (cc_list[i].isStar()) {
      markers[i + 1] = operation::marker::match_star(markers[i], cc_bit_streams[i]);
    } else {
      markers[i + 1] = operation::marker::advance(markers[i], cc_bit_streams[i]);
    }
  }

  
  std::cout << "matched = " << markers.back().pop_count() << std::endl;

  auto tock = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time = tock - tick;
  std::cout << "elapsed time = " << elapsed_time.count() << " second" << std::endl;
  
  return 0;
}
