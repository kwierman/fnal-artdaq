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

    std::size_t size() const
    { return vals_.size(); }

    void resize(std::size_t sz, RawDataType v = RawDataType())
    { vals_.resize(sz, v); }

    iterator begin()
    { return vals_.begin(); }

    iterator end()
    { return vals_.end(); }

    const_iterator begin() const
    { return vals_.begin(); }

    const_iterator end() const
    { return vals_.end(); }

    RawDataType& operator[](std::size_t i)
    { return vals_[i]; }

    RawDataType operator[](std::size_t i) const
    { return vals_[i]; }

    void clear()
    { vals_.clear(); }

    bool empty()
    { return vals_.empty(); }

    void reserve(std::size_t cap)
    { vals_.reserve(cap); }

    void push_back(RawDataType v)
    { vals_.push_back(v); }

    void swap(Fragment& other)
    { vals_.swap(other.vals_); }

    RawFragmentHeader const* fragmentHeader() const
    { return reinterpret_cast<RawFragmentHeader const*>(&vals_[0]); }

  private:
    std::vector<RawDataType> vals_;
  };

  inline void swap(Fragment& x, Fragment& y) { x.swap(y); }

  typedef std::vector<uint64_t>           CompressedFragPart;
  typedef std::vector<CompressedFragPart> CompressedFragParts;

  struct DarkSideHeaderOverlay
  {
    unsigned long event_size_ : 28;
    unsigned long junk1 : 4;
    unsigned long channel_mask_ : 8;
    unsigned long pattern_ : 16;
    unsigned long junk2 : 3;
    unsigned long board_id_ : 5;
    unsigned long event_counter_ : 24;
    unsigned long reserved_ : 8;
    unsigned long trigger_time_tag_ : 32;
  };

  struct DarkSideHeader
  {
    unsigned long word0;
    unsigned long word1;
  };

  struct CompressedBoard
  {
    DarkSideHeader header_;
    CompressedFragParts parts_;
  };

  typedef std::vector<CompressedBoard> CompressedRawEvent;
}
#endif
