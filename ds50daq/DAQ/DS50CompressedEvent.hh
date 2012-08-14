#ifndef artdaq_DAQdata_DS50CompressedEvent_hh
#define artdaq_DAQdata_DS50CompressedEvent_hh

// NOTE: the GPU might be more efficient at using 32-bit integers than 64-bit integers,
// in this case the code below will need to be modified.

// What size do we want the compressed vectors? resized to the amount of data.
// Should we also store a vector of compressed fragment lengths? yes, because the
// total bits returned from the encoders is an important number

#include "ds50daq/DAQ/DS50Types.hh"
#include "artdaq/DAQdata/features.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "ds50daq/DAQ/detail/DS50Header.hh"

#include <vector>

namespace ds50 {
  class CompressedEvent;
}

class ds50::CompressedEvent {
public:
  CompressedEvent() { }
  // will set up the headers and the sizes given a set of fragments
  explicit CompressedEvent(std::vector<artdaq::Fragment> const & init);

  DataVec & fragment(size_t which)
    { return compressed_fragments_.at(which); }
  DataVec const & fragment(size_t which) const
    { return compressed_fragments_.at(which); }

  reg_type fragmentBitCount(size_t which) const
    { return counts_.at(which); }
  void setFragmentBitCount(size_t which, reg_type count)
    { counts_[which] = count; }

  size_t size() const { return compressed_fragments_.size(); }

  // return a reference to the entire CompVec? perhaps.

#if USE_MODERN_FEATURES
  // since structures for headers are in the details, there is
  // no other clean way to present them here to the user.
  artdaq::Fragment headerOnlyFrag(size_t which) const;
#endif

  // Needs to be public for ROOT persistency: do not use.
  struct HeaderProxy {
    detail::Header::data_t hp[detail::Header::size_words];
  };

private:
  // ROOT persistency can't handle bitfields.
  typedef std::vector<HeaderProxy> DS50HeaderVec;
  typedef std::vector<DataVec> CompVec;
  typedef std::vector<reg_type> CountVec;

  DS50HeaderVec ds50_headers_;
  CompVec compressed_fragments_;
  CountVec counts_;
};

#if USE_MODERN_FEATURES
#include "ds50daq/DAQ/DS50Board.hh"

inline
artdaq::Fragment
ds50::CompressedEvent::headerOnlyFrag(size_t which) const
{
  using artdaq::Fragment;
  Fragment result
    (Fragment::dataFrag(Fragment::InvalidSequenceID,
                        Fragment::InvalidFragmentID,
                        reinterpret_cast<Fragment::value_type const *>
                        (&ds50_headers_.at(which)),
                        reinterpret_cast<Fragment::value_type const *>
                        (&ds50_headers_.at(which) +
                         detail::Header::size_words)));
  ds50::Board b(result);
  result.setSequenceID(b.event_counter());
  result.setFragmentID(b.board_id());
  return result;
}
#endif /* USE_MODERN_FEATURES */

#endif /* artdaq_DAQdata_DS50CompressedEvent_hh */
