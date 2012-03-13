#ifndef artdaq_DAQdata_detail_RawFragmentHeader_hh
#define artdaq_DAQdata_detail_RawFragmentHeader_hh
// detail::RawFragmentHeader is an overlay that provides the user's view
// of the data contained within a Fragment. It is intended to be hidden
// from the user of Fragment, as an implementation detail. The interface
// of Fragment is intended to be used to access the data.

#include "artdaq/DAQdata/features.hh"

namespace detail {
  struct RawFragmentHeader;
}

struct detail::RawFragmentHeader {
  typedef uint64_t RawDataType;

#if USE_MODERN_FEATURES
  typedef uint16_t version_t;
  typedef uint64_t event_id_t;
  enum type_t : uint8_t {
    DATA = 0,
    END_OF_DATA,
    INVALID = 0xFF
  };
  typedef uint16_t fragment_id_t;

  // Each of the following invalid values is chosen based on the
  // size of the bitfield in which the corresponding data are
  // encoded; if any of the sizes are changed, the corresponding
  // values must be updated.
  static const version_t InvalidVersion  = 0xFFFF;
  static const event_id_t InvalidEventID = 0xFFFFFFFFFFFF;
  static const fragment_id_t InvalidFragmentID = 0xFFFF;

  RawDataType word_count  : 32; // number of RawDataTypes in this Fragment
  RawDataType version     : 16;
  RawDataType type        :  8;
  RawDataType unused      :  8;

  RawDataType event_id    : 48;
  RawDataType fragment_id : 16;

  constexpr static std::size_t num_words()
  { return sizeof(detail::RawFragmentHeader); }

#endif /* USE_MODERN_FEATURES */

};

#endif /* artdaq_DAQdata_detail_RawFragmentHeader_hh */
