#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/ast.h"
#include "codegen/expression_compiler_cpp.h"
#include "codegen/expression_compiler_llvm.h"
#include "stream/cc_bit_stream.h"
#include "stream/marker_bit_stream.h"
#include "operations/marker.h"

void print_help() {
  std::cerr << "usage: program [input] [regex]" << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    print_help();
    exit(0);
  }
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

  parser::ReParser parser;
  codegen::CCCompiler cc_compiler;

  auto input = std::string(argv[1]);
  auto cc_list = parser.parse(argv[2]);
  auto input_size = input.length();
  auto cc_size = cc_list.size();

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
  std::vector<stream::CCBitStream> cc_bit_streams(cc_size, stream::CCBitStream(input_size));
  for (size_t i = 0; i < input_size; ++i) {
    std::vector<uint8_t> match_result(cc_size);
    expr_compiler.run(input[i], reinterpret_cast<uint8_t*>(match_result.data()));
    for (size_t j = 0; j < cc_size; ++j) {
      cc_bit_streams[j].set(i, match_result[j]);
    }
  }

  auto cc_width = 12;
  int width = static_cast<int>(input_size + cc_width);
  std::cout << std::setw(width) << std::right << input << std::endl;
  for (size_t i = 0; i < cc_size; ++i) {
    auto& cc = cc_list[i];
    std::cout << std::setw(cc_width) << std::left << cc;
    std::cout << cc_bit_streams[i] << std::endl;
  }
  
  // Marker bit streams
  auto markers_size = cc_size + 1;
  std::vector<stream::MarkerBitStream> markers(markers_size, stream::MarkerBitStream(input_size));
  markers[0] = cc_bit_streams[0];
  for (size_t i = 1; i < markers_size; ++i) {
    std::cout << std::setw(cc_width) << std::left << "markers " + std::to_string(i);
    if (cc_list[i - 1].isStar()) {
      markers[i] = operation::marker::match_star(markers[i - 1], cc_bit_streams[i - 1]);
    } else {
      markers[i] = operation::marker::advance(markers[i - 1], cc_bit_streams[i - 1]);
    }
    std::cout << markers[i] << std::endl;
  }

    }
  }
  
  return 0;
}
