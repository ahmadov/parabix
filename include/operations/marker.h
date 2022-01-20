#include "stream/bit_stream.h"

using BitStream = stream::BitStream;

namespace operation {

  namespace marker {

    inline BitStream advance(const BitStream& marker, BitStream& cc) {
      BitStream result = marker;
      result &= cc;
      result >>= 1;
      return result;
    }

    inline BitStream match_star(const BitStream& marker, BitStream& cc) {
      BitStream result = marker;
      result &= cc;
      result += cc;
      result ^= cc;
      result |= marker;
      return result;
    }

    inline BitStream scan_thru(const BitStream& marker, BitStream& cc) {
      BitStream result = marker;
      result += cc;
      result &= ~cc;
      return result;
    }


  } // namespace marker

} // namespace operation
