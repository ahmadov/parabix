#include "stream/cc_bit_stream.h"
#include "stream/marker_bit_stream.h"

using CCBitStream = stream::CCBitStream;
using MarkerBitStream = stream::MarkerBitStream;

namespace operation {

  namespace marker {

    inline MarkerBitStream advance(const MarkerBitStream& marker, const CCBitStream& cc) {
      MarkerBitStream result = marker;
      result &= cc;
      result >>= 1;
      return result;
    }

    inline MarkerBitStream match_star(const MarkerBitStream& marker, const CCBitStream& cc) {
      MarkerBitStream result = marker;
      result &= cc;
      result += cc;
      result ^= cc;
      result |= marker;
      return result;
    }

    inline MarkerBitStream scan_thru(const MarkerBitStream& marker, const CCBitStream& cc) {
      MarkerBitStream result = marker;
      result += cc;
      result &= ~cc;
      return result;
    }


  } // namespace marker

} // namespace operation
