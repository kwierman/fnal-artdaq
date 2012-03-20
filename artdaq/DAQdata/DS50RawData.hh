#ifndef DS50RAWDATA_H
#define DS50RAWDATA_H 1

// NOTE: the GPU might be more efficient at using 32-bit integers than 64-bit integers,
// in this case the code below will need to be modified.

// What size do we want the compressed vectors? resized to the amount of data.
// Should we also store a vector of compressed fragment lengths? yes, because the
// total bits returned from the encoders is an important number

#include "artdaq/Compression/Properties.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/detail/DS50Header.hh"

#include <vector>

namespace ds50
{
  class DS50RawData
  {
  public:
    DS50RawData();
    // will set up the headers and the sizes given a set of fragments
    explicit DS50RawData(std::vector<artdaq::Fragment> const& init);

    DataVec& fragment(size_t which)
    { return compressed_fragments_.at(which); }
    DataVec const& fragment(size_t which) const
    { return compressed_fragments_.at(which); }

    reg_type fragmentBitCount(size_t which) const
    { return counts_.at(which); }
    void setFragmentBitCount(size_t which, reg_type count)
    { counts_[which] = count; }

    // return a reference to the entire CompVec? perhaps.

    // since structures for headers are in the details, there is
    // no clean way to present them here to the user.
    
  private:
    typedef std::vector<detail::Header> DS50HeaderVec;
    // typedef std::vector<artdaq::detail::RawFragmentHeader> FragHeaderVec;
    typedef std::vector<DataVec> CompVec;
    typedef std::vector<reg_type> CountVec;

    DS50HeaderVec ds50_headers_;
    CompVec compressed_fragments_;
    CountVec counts_;
    // FragHeaderVec frag_headers_;
  };
}

#endif
