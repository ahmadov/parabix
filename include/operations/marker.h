#include "stream/basis_bit_stream.h"

using BasisBitStream = stream::BasisBitStream;

namespace operation {

  namespace marker {

    inline BasisBitStream advance(const BasisBitStream& marker, const BasisBitStream& cc) {
      BasisBitStream result = marker;
      result &= cc;
      result >>= 1;
      return result;
    }

    inline BasisBitStream match_star(const BasisBitStream& marker, const BasisBitStream& cc) {
      BasisBitStream result = marker;
      result &= cc;
      result += cc;
      result ^= cc;
      result |= marker;
      return result;
    }

    inline BasisBitStream scan_thru(const BasisBitStream& marker, const BasisBitStream& cc) {
      BasisBitStream result = marker;
      result += cc;
      result &= ~cc;
      return result;
    }


  } // namespace marker

} // namespace operation
