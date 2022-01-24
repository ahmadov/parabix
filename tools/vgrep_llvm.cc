#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <chrono> // NOLINT
#include <immintrin.h>
#include <llvm-c/Target.h>
#include <llvm/Support/DynamicLibrary.h>
#include "PerfEvent.hpp"
#include "parabix/parabix.h"

void print_help(const char* name) {
  std::cerr << "usage: " << name << " [/path/to/file] [regex]" << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    print_help(argv[0]);
    exit(0);
  }

  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);

  std::ifstream t(argv[1]);
  std::stringstream buffer;
  buffer << t.rdbuf();

  auto input = buffer.str();
  auto pattern = argv[2];

  auto tick = std::chrono::high_resolution_clock::now();

  llvm::orc::ThreadSafeContext context(std::make_unique<llvm::LLVMContext>());

  PerfEvent e;
  e.startCounters();

  std::cout << "matched = " << parabix::parabix_llvm(context, input, pattern) << std::endl;

  e.stopCounters();
  e.printReport(std::cout, input.size()); // use n as scale factor

  auto tock = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time = tock - tick;
  std::cout << "elapsed time = " << elapsed_time.count() << " second" << std::endl;
  
  return 0;
}
