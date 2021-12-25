#include <iostream>
#include <iomanip>
#include "codegen/cc_compiler.h"
#include "parser/re_parser.h"

void print_help() {
  std::cerr << "usage: program [regex]" << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    print_help();
    exit(0);
  }

  parser::ReParser parser(argv[1]);
  codegen::CCCompiler compiler;

  auto cc_list = parser.getCCs();
  for (auto& cc : cc_list) {
    auto expression = compiler.createBitwiseExpression(cc);
    std::cout << std::setw(10) << std::left << cc << " => " << expression->as_string() << std::endl;
  }
  
  return 0;
}
