#ifndef artdaq_DAQdata_detail_RawFragmentHeader_hh
#define artdaq_DAQdata_detail_RawFragmentHeader_hh
// detail::RawFragmentHeader is an overlay that provides the user's view
// of the data contained within a Fragment. It is intended to be hidden
// from the user of Fragment, as an implementation detail. The interface
// of Fragment is intended to be used to access the data.

#include <cstddef>
#include "artdaq/DAQdata/features.hh"

extern "C" {
#include <stdint.h>
}

namespace artdaq {
  namespace detail {
    struct RawFragmentHeader;
  }
}

struct artdaq::detail::RawFragmentHeader {
  typedef uint64_t RawDataType;

#if USE_MODERN_FEATURES
  typedef uint16_t version_t;
  typedef uint64_t sequence_id_t;
  typedef uint8_t  type_t;
  typedef uint16_t fragment_id_t;

  // define reserved values for type_t
  static const type_t InvalidFragmentType = 0;
  static const type_t EndOfDataFragmentType= 1;
  static const type_t DataFragmentType = 2;
  // experiment-specific fragment types: 16-255

  // Each of the following invalid values is chosen based on the
  // size of the bitfield in which the corresponding data are
  // encoded; if any of the sizes are changed, the corresponding
  // values must be updated.
  static const version_t InvalidVersion  = 0xFFFF;
  static const sequence_id_t InvalidSequenceID = 0xFFFFFFFFFFFF;
  static const fragment_id_t InvalidFragmentID = 0xFFFF;

  RawDataType word_count  : 32; // number of RawDataTypes in this Fragment
  RawDataType version     : 16;
  RawDataType type        :  8;
  RawDataType unused      :  8;

  RawDataType sequence_id : 48;
  RawDataType fragment_id : 16;

  constexpr static std::size_t num_words();

#endif /* USE_MODERN_FEATURES */

};

#if USE_MODERN_FEATURES
inline
constexpr
std::size_t
artdaq::detail::RawFragmentHeader::num_words()
{
  return sizeof(detail::RawFragmentHeader) / sizeof(RawDataType);
}
#endif

#endif /* artdaq_DAQdata_detail_RawFragmentHeader_hh */
