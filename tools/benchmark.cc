#include <bits/stdc++.h>
#include <fstream>
#include <regex>
#include <llvm-c/Target.h>
#include <llvm/Support/DynamicLibrary.h>
#include "parabix/parabix.h"

uint64_t std_regex(std::string& input, const char* pattern) {
  std::regex r1(pattern);
  auto words_begin = std::sregex_iterator(input.begin(), input.end(), r1);
  auto words_end = std::sregex_iterator();

  return std::distance(words_begin, words_end);
}

int main(int argc, char** argv) {

  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  llvm::orc::ThreadSafeContext context(std::make_unique<llvm::LLVMContext>());

  auto pattern = "a[0-9]*z";
  std::vector<std::string> files = {"10mb.txt", "100mb.txt", "500mb.txt", "1gb.txt"};
  std::vector<std::string> algorithms = {"std::regex", "parabix-cpp", "parabix-llvm"};
  std::vector<std::vector<double>> elapsed_times(files.size(), std::vector<double>(algorithms.size()));

  for (auto i = 0; i < files.size(); ++i) {
    std::ifstream t("../input/" + files[i]);
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto input = buffer.str();
    for (auto j = 0; j < algorithms.size(); ++j) {
      auto tick = std::chrono::high_resolution_clock::now();

      if (algorithms[j] == "std::regex") {
        std_regex(input, pattern);
      } else if (algorithms[j] == "parabix-cpp") {
        parabix::parabix_cpp(input, pattern);
      } else {
        parabix::parabix_llvm(context, input, pattern);
      }

      auto tock = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed_time = tock - tick;
      elapsed_times[i][j] = elapsed_time.count();
    }
  }

  std::cout << std::setw(10) << std::left << "size/algo";
  for (auto& algo : algorithms) {
    std::cout << std::setw(20) << std::right << algo;
  }
  std::cout << std::endl;
  for (auto i = 0; i < files.size(); ++i) {
    std::cout << std::setw(10) << std::left << files[i];
    for (auto j = 0; j < algorithms.size(); ++j) {
      std::cout << std::setw(20) << std::right << std::setprecision(2) << elapsed_times[i][j];
    }
    std::cout << std::endl;
  }

  return 0;
}
