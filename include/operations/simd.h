#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <immintrin.h>

namespace operation {

  namespace simd {

    inline std::vector<uint8_t> advance(const std::vector<uint8_t>& marker, const std::vector<uint8_t>& cc) {
      assert(marker.size() == cc.size() && "sizes must be same");

      auto size = cc.size();
      auto simd_size = 32;
      std::vector<uint8_t> result(size);
      size_t near = size - (size % simd_size);

      for (auto i = 0; i < near; i += simd_size) {
        const __m256i* marker_ptr = reinterpret_cast<const __m256i*>(marker.data() + i);
        const __m256i* cc_ptr = reinterpret_cast<const __m256i*>(cc.data() + i);
        __m256i marker_i = _mm256_loadu_si256(marker_ptr);
        __m256i cc_i = _mm256_loadu_si256(cc_ptr);

        __m256i op1 = _mm256_and_si256(marker_i, cc_i);

        __m256i* result_i = reinterpret_cast<__m256i*>(result.data() + i);
        _mm256_storeu_si256(result_i, op1);
      }

      for (auto i = near; i < size; ++i) {
        result[i] = marker[i] & cc[i];
      }

      memmove(result.data() + 1, result.data(), size - 1);
      auto carry = result[size - 1] == 1;
      result[0] = 0;

      return result;
    }

    inline std::vector<uint8_t> match_star(const std::vector<uint8_t>& marker, const std::vector<uint8_t>& cc) {
      assert(marker.size() == cc.size() && "sizes must be same");

      auto size = cc.size();
      auto simd_size = 32; // 256 / 8
      std::vector<uint8_t> result(size);
      size_t near = size - (size % simd_size);

      for (auto i = 0; i < near; i += simd_size) {
        const __m256i* marker_ptr = reinterpret_cast<const __m256i*>(marker.data() + i);
        const __m256i* cc_ptr = reinterpret_cast<const __m256i*>(cc.data() + i);
        __m256i marker_i = _mm256_loadu_si256(marker_ptr);
        __m256i cc_i = _mm256_loadu_si256(cc_ptr);

        // 1. AND
        __m256i op1 = _mm256_and_si256(marker_i, cc_i);

        __m256i* result_i = reinterpret_cast<__m256i*>(result.data() + i);
        _mm256_storeu_si256(result_i, op1);
      }

      for (auto i = near; i < size; ++i) {
        result[i] = marker[i] & cc[i];
      }

      std::vector<uint8_t> carry(size, 0);
      for (auto i = 0; i < size; ++i) {
        auto op1 = result[i];

        // 2. ADD
        auto op2 = ((op1 ^ cc[i]) ^ carry[i]);
        carry[i + 1] = ((op1 & cc[i]) | (op1 & carry[i])) | (cc[i] & carry[i]); 

        result[i] = op2;
      }

      for (auto i = 0; i < near; i += simd_size) {
        const __m256i* marker_ptr = reinterpret_cast<const __m256i*>(marker.data() + i);
        const __m256i* cc_ptr = reinterpret_cast<const __m256i*>(cc.data() + i);
        __m256i marker_i = _mm256_loadu_si256(marker_ptr);
        __m256i cc_i = _mm256_loadu_si256(cc_ptr);

        __m256i* result_i = reinterpret_cast<__m256i*>(result.data() + i);
        __m256i op2 = _mm256_loadu_si256(result_i);

        // 3. XOR
        __m256i op3 = _mm256_xor_si256(op2, cc_i);

        // 4. OR
        __m256i op4 = _mm256_or_si256(op3, marker_i);

        _mm256_storeu_si256(result_i, op4);
      }

      for (auto i = near; i < size; ++i) {
        auto op2 = result[i];

        // 3. XOR
        auto op3 = op2 ^ cc[i];

        // 4. OR
        auto op4 = op3 | marker[i];

        result[i] = op4;
      }

      return result;
    }

  }; // namespace simd

}; // namespace operation
