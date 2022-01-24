#include <cstdint>
#include <emmintrin.h>

namespace parabix {
  
  // simplified version of https://mischasan.wordpress.com/2011/10/03/the-full-sse2-bit-matrix-transpose-routine/ 
  inline void transpose_sse(char *inp, uint8_t *out) {
    int rr, i, h;
    union { __m128i x; uint8_t b[16]; } tmp{};

    // Do the main body in 16x8 blocks:
    for (rr = 0; rr < 4; ++rr) {
      for (i = 0; i < 16; ++i) {
        tmp.b[i] = inp[rr * 16 + i];
      }
      for (i = 0; i < 8; ++i, tmp.x = _mm_slli_epi64(tmp.x, 1)) {
        *reinterpret_cast<uint16_t*>(&out[(rr * 2) + (i * 8)]) = _mm_movemask_epi8(tmp.x);
      }
    }
  }

} // namespace parabix
