#include <iostream>
#include <iomanip>
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/expression_compiler_cpp.h"

void print_help() {
  std::cerr << "usage: program [regex]" << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    print_help();
    exit(0);
  }

  parser::ReParser parser(argv[1]);
  codegen::CCCompiler cc_compiler;
  codegen::ExpressionCompilerCpp expr_compiler_cpp;

  auto cc_list = parser.getCCs();
  for (auto& cc : cc_list) {
    auto expression = cc_compiler.compile(cc);
    std::cout << std::setw(10) << std::left << cc << " => " << expression->as_string() << std::endl;

    std::cout << expr_compiler_cpp.compile(*expression) << std::endl;
  }
  
  return 0;
}
