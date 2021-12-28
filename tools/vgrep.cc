#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/expression_compiler_cpp.h"
#include "codegen/expression_compiler_llvm.h"

void print_help() {
  std::cerr << "usage: program [regex]" << std::endl;
}

std::string join_vec(std::vector<char>& arr) {
  return std::accumulate(
    arr.cbegin(),
    arr.cend(),
    std::string(""),
    [](std::string result, auto ch) {
      return std::move(result) + ", " + ch;
    }
  );
}

int main(int argc, char** argv) {
  if (argc != 2) {
    print_help();
    exit(0);
  }
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

  parser::ReParser parser(argv[1]);
  codegen::CCCompiler cc_compiler;
  codegen::ExpressionCompilerCpp expr_compiler_cpp;

  llvm::orc::ThreadSafeContext context(std::make_unique<llvm::LLVMContext>());
  codegen::ExpressionCompiler expr_compiler(context);

  auto cc_list = parser.getCCs();
  for (auto& cc : cc_list) {
    auto expression = cc_compiler.compile(cc);
    std::cout << std::setw(10) << std::left << cc << " => " << expression->as_string() << std::endl;

    std::cout << expr_compiler_cpp.compile(*expression) << std::endl;

    expr_compiler.compile(*expression, true);

    std::vector<char> matched;
    std::vector<char> unmatched;
    for (auto c = '0'; c <= 'z'; ++c) {
      if (expr_compiler.run(c)) {
        matched.push_back(c);
      } else {
        unmatched.push_back(c);
      }
    }
    std::cout << "MATCHED: " << join_vec(matched) << std::endl;
    std::cout << "UNMATCHED: " << join_vec(unmatched) << std::endl;
  }
  
  return 0;
}
