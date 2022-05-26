#include <bits/stdc++.h>
#include <fstream>
#include "parser/re_parser.h"

namespace {

// A function to return a seeded random number generator.
inline std::mt19937& generator() {
    // the generator will only be seeded once (per thread) since it's static
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

// A function to generate integers in the range [min, max]
template<typename T, std::enable_if_t<std::is_integral_v<T>>* = nullptr>
T rand(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(generator());
}

// A function to generate floats in the range [min, max)
template<typename T, std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
T rand(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
    std::uniform_real_distribution<T> dist(min, max);
    return dist(generator());
}

// Uniform random number
int32_t URand(int32_t min, int32_t max) {
   return rand<int32_t>(min, max);
}

} // namespace

const int32_t times = 5000;

struct GeneratorInput {
  const char* pattern;
  bool star = false;
  bool random = false;
  int32_t max_count = 1;
};

char generate_random_character_between_range(int32_t start, int32_t end) {
  return char(URand(start, end));
}

void generate(const std::vector<GeneratorInput>& generator_inputs, std::vector<std::string>& output) {
  parser::ReParser parser;
  for (auto t = 0; t < times; ++t) {
    std::string generated;
    for (auto& generator_input : generator_inputs) {
      auto generate = generator_input.random ? URand(0, 1) : true;
      if (generate) {
        auto cc_list = parser.parse(generator_input.pattern);
        // TODO(elmi): handle multiple ranges?
        auto ranges = cc_list[0].getRanges();
        auto random_index = URand(0, static_cast<int32_t>(ranges.size() - 1)); 
        auto& range = ranges[random_index];
        if (generator_input.star) {
          auto count = URand(0, generator_input.max_count);
          while (count--) {
            generated.push_back(generate_random_character_between_range(range.first, range.second));
          }
        } else {
          generated.push_back(generate_random_character_between_range(range.first, range.second));
        }
      }
    }
    output.emplace_back(generated);
  }
}

void generate_pattern(std::ofstream& output, int32_t size_in_mb) {
  // a[0-9]*z
  std::vector<GeneratorInput> always_matching_pattern = {
    {.pattern = "[a-zA-Z]", .star  = false, .random = true},
    {.pattern = "a", .star  = false, .random = false},
    {.pattern = "[0-9]", .star  = true, .max_count = 5},
    {.pattern = "z", .star  = false, .random = false},
    {.pattern = "[a-zA-Z]", .star  = false, .random = true},
    {.pattern = "_", .star  = false, .random = true},
  };
  std::vector<GeneratorInput> randomly_matching_pattern = {
    {.pattern = "[a-zA-Z]", .star  = false, .random = true},
    {.pattern = "[a-d]", .star  = false, .random = false},
    {.pattern = "[0-9]", .star  = true, .max_count = 5},
    {.pattern = "[x-z]", .star  = false, .random = false},
    {.pattern = "[a-zA-Z]", .star  = false, .random = true},
    {.pattern = "_", .star  = false, .random = true},
  };
  std::vector<GeneratorInput> never_matching_pattern = {
    {.pattern = "[b-zA-Z]", .star  = false, .random = true},
    {.pattern = "[b-d]", .star  = false, .random = false},
    {.pattern = "[0-9]", .star  = true, .max_count = 5},
    {.pattern = "[x-y]", .star  = false, .random = false},
    {.pattern = "[a-yA-Z]", .star  = false, .random = true},
    {.pattern = "_", .star  = false, .random = true},
  };

  std::vector<std::string> generated_strings;
  generate(always_matching_pattern, generated_strings);
  generate(randomly_matching_pattern, generated_strings);
  generate(never_matching_pattern, generated_strings);

  int32_t required_size_in_bytes = size_in_mb * 1024 * 1024;
  int32_t generated_size_in_bytes = 0;
  for (auto& generated_str : generated_strings) {
    generated_size_in_bytes += static_cast<int32_t>(generated_str.size());
  }
  auto at_least = required_size_in_bytes / generated_size_in_bytes;
  for (auto i = 0; i < at_least; ++i) {
    for (auto& generated_str : generated_strings) output << generated_str;
  }
  auto total_size = at_least * generated_size_in_bytes;
  auto left = required_size_in_bytes - total_size;
  for (auto& generated_str : generated_strings) {
    if (left <= 0) {
      break;
    }
    output << generated_str;
    auto size = static_cast<int32_t>(generated_str.size());
    left -= size;
    total_size += size;
  };
  for (; total_size % 63; total_size++) {
    output << "-";
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << argv[0] << " [size_in_mb] [output file]";
    exit(1);
  }

  auto size_in_mb = atoi(argv[1]);
  std::ofstream output_stream(argv[2]);

  generate_pattern(output_stream, size_in_mb); // a[0-9]*z

  output_stream.close();

  return 0;
}
