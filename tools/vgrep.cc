#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include "parser/re_parser.h"
#include "codegen/cc_compiler.h"
#include "codegen/ast.h"
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

  parser::ReParser parser;
  codegen::CCCompiler cc_compiler;

  auto cc_list = parser.parse(argv[1]);

  llvm::orc::ThreadSafeContext context(std::make_unique<llvm::LLVMContext>());
  std::vector<std::unique_ptr<codegen::BitwiseExpression>> expressions;
  for (auto& cc : cc_list) {
    auto expression = cc_compiler.compile(cc);
    std::cout << std::setw(10) << std::left << cc << " => " << expression->as_string() << std::endl;

    codegen::ExpressionCompilerCpp expr_compiler_cpp;
    std::cout << expr_compiler_cpp.compile(*expression) << std::endl;

    expressions.push_back(std::move(expression));
  }

  codegen::ExpressionCompiler expr_compiler(context);
  expr_compiler.compile(expressions, true);
  std::vector<char> matched;
  std::vector<char> unmatched;
  std::vector<uint8_t> cc_match(cc_list.size());
  for (auto c = '0'; c <= 'z'; ++c) {
    expr_compiler.run(c, reinterpret_cast<uint8_t*>(cc_match.data()));
    uint8_t match = 0;
    for (auto i = 0; i < cc_list.size(); ++i) {
      match |= cc_match[i] & 1;
    }
    if (match) {
      matched.push_back(c);
    } else {
      unmatched.push_back(c);
    }
  }
  std::cout << "MATCHED: " << join_vec(matched) << std::endl;
  std::cout << "UNMATCHED: " << join_vec(unmatched) << std::endl;
  
  return 0;
}
