// ---------------------------------------------------------------------------
#ifndef INCLUDE_STREAM_MARKER_BIT_STREAM_H_
#define INCLUDE_STREAM_MARKER_BIT_STREAM_H_
// ---------------------------------------------------------------------------
#include "stream/basis_bit_stream.h"
#include "stream/cc_bit_stream.h"
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
namespace stream {
// ---------------------------------------------------------------------------
// Marker Bit Stream
class MarkerBitStream: public BasisBitStream {
  public:
    explicit MarkerBitStream(size_t length)
      : BasisBitStream(length) {} 

    MarkerBitStream& operator=(const CCBitStream& other) {
      blocks = other.getBlocks();
      return *this;
    }
};
// ---------------------------------------------------------------------------
} // namespace stream
// ---------------------------------------------------------------------------
#endif  // INCLUDE_STREAM_MARKER_BIT_STREAM_H_
// ---------------------------------------------------------------------------
