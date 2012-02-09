#ifndef DAQ_FRAGMENT_HH
#define DAQ_FRAGMENT_HH

#include <cstddef>
#include <vector>
#include <stdint.h>

#include "DAQdata/RawData.hh"

namespace artdaq
{
  class Fragment
  {
  public:

    typedef std::vector<RawDataType>::reference      reference;
    typedef std::vector<RawDataType>::iterator       iterator;
    typedef std::vector<RawDataType>::const_iterator const_iterator;
    typedef std::vector<RawDataType>::value_type     value_type;

    Fragment();
    explicit Fragment(std::size_t n);

#if !defined(__GCCXML__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
    // Return the number of words in the data payload. This does not
    // include the number of words in the header.
    std::size_t dataSize() const
    { return vals_.size() - data_offset_(); }

    // Resize the data payload to hold sz words.
    void resize(std::size_t sz, RawDataType v = RawDataType())
    { vals_.resize(sz+data_offset_(), v); }

    // Return an iterator to the beginning of the data payload.
    iterator dataBegin()
    { return vals_.begin() + data_offset_(); }

    iterator dataEnd()
    { return vals_.end(); }

    const_iterator dataBegin() const
    { return vals_.begin() + data_offset_(); }

    const_iterator dataEnd() const
    { return vals_.end(); }

    RawDataType& operator[](std::size_t i)
    { return vals_[i+data_offset_()]; }

    RawDataType operator[](std::size_t i) const
    { return vals_[i+data_offset_()]; }

    void clear()
    { vals_.erase(dataBegin(), dataEnd()); }

    bool empty()
    { return vals_.size() - data_offset_() == 0; }

    void reserve(std::size_t cap)
    { vals_.reserve(cap + data_offset_()); }

    void push_back(RawDataType v)
    { vals_.push_back(v); }

    void swap(Fragment& other)
    { vals_.swap(other.vals_); }

    RawFragmentHeader* fragmentHeader()
    { return reinterpret_cast<RawFragmentHeader*>(&vals_[0]); }

    RawFragmentHeader const* fragmentHeader() const
    { return reinterpret_cast<RawFragmentHeader const*>(&vals_[0]); }
#endif

  private:
#if !defined(__GCCXML__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
    constexpr static std::size_t data_offset_() 
    { return RawFragmentHeader::num_vals(); }
#endif

    std::vector<RawDataType> vals_;
  };

#if !defined(__GCCXML__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
  inline void swap(Fragment& x, Fragment& y) { x.swap(y); }
#endif

  typedef std::vector<uint64_t>           CompressedFragPart;
  typedef std::vector<CompressedFragPart> CompressedFragParts;


}
#endif
