#include <bits/stdc++.h>
#include <fstream>
#include <regex>
#include <llvm-c/Target.h>
#include <llvm/Support/DynamicLibrary.h>
#include "parabix/parabix.h"

struct State {
  std::unordered_map<uint8_t, uint8_t> next;
  bool final = false;

  State() {
    for (auto i = 0; i < 256; ++i) {
      next[i] = 0;
    }
  }
};

uint64_t std_regex(std::string& input, const char* pattern) {
  std::regex r1(pattern);
  auto words_begin = std::sregex_iterator(input.begin(), input.end(), r1);
  auto words_end = std::sregex_iterator();

  return std::distance(words_begin, words_end);
}

uint64_t DFA(std::string& input, const char* pattern) {
  assert(strcmp(pattern, "a[0-9]*z") == 0 && "DFA can only handle 'a[0-9]*z'");
  // pattern: a[0-9]*z
  std::vector<State> states(4);
  states[3].final = true;

  // current = root or first char 'a' and next -> a
  states[0].next['a'] = 1;
  states[1].next['a'] = 1;

  // current = 'a' and next -> [0-9]* or z
  for (char c = '0'; c <= '9'; ++c) {
    states[1].next[c] = 2; // next -> [0-9]*
  }
  // z
  states[1].next['z'] = 3; // next -> z

  // current = '[0-9]' and next -> [0-9]* or z
  states[2].next['a'] = 1;
  for (char c = '0'; c <= '9'; ++c) {
    states[2].next[c] = 2; // next -> [0-9]*
  }
  // z
  states[2].next['z'] = 3; // next -> z

  uint64_t matched = 0, current_state_idx = 0;
  for (auto& c : input) {
    current_state_idx = states[current_state_idx].next[c];
    if (states[current_state_idx].final) {
      current_state_idx = 0;
      matched++;
    }
  }
  return matched;
}

int main(int argc, char** argv) {

  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  llvm::orc::ThreadSafeContext context(std::make_unique<llvm::LLVMContext>());

  auto pattern = "a[0-9]*z";
  std::vector<std::string> files = {"10mb.txt", "50mb.txt", "100mb.txt", "500mb.txt", "1gb.txt"};
  std::vector<std::string> algorithms = {"std::regex", "DFA", "parabix-cpp", "parabix-llvm"};
  std::vector<std::vector<double>> elapsed_times(files.size(), std::vector<double>(algorithms.size()));

  for (auto i = 0; i < files.size(); ++i) {

    auto rf_tick = std::chrono::high_resolution_clock::now();
    std::ifstream t("../input/" + files[i]);
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto input = buffer.str();
    std::vector<uint64_t> results;
    for (auto j = 0; j < algorithms.size(); ++j) {
      auto tick = std::chrono::high_resolution_clock::now();

      if (algorithms[j] == "std::regex") {
        results.push_back(std_regex(input, pattern));
      } else if (algorithms[j] == "DFA") {
        results.push_back(DFA(input, pattern));
      } else if (algorithms[j] == "parabix-cpp") {
        results.push_back(parabix::parabix_cpp(input, pattern));
      } else {
        results.push_back(parabix::parabix_llvm(context, input, pattern, false));
      }

      auto tock = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed_time = tock - tick;
      elapsed_times[i][j] = elapsed_time.count();
    }

    for (auto l = 0; l < results.size() - 1; ++l) {
      if (results[l] != results[l + 1]) {
        std::cerr << "results must be same - " << l << " = " << l + 1 << std::endl;
        exit(1);
      }
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
