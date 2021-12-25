#include <iostream>
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
    std::cout << cc << std::endl;
  }
  compiler.compile(cc_list);
  
  return 0;
}
