#ifndef artdaq_DAQdata_Fragment_hh
#define artdaq_DAQdata_Fragment_hh

#include <cstddef>
#include <iosfwd>
#include <vector>
#include <stdint.h>

#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"
#include "artdaq/DAQdata/features.hh"

namespace artdaq
{
  typedef detail::RawFragmentHeader::RawDataType RawDataType;

  class Fragment
  {
  public:
    // Create a Fragment with all header values zeroed.
    Fragment();

    // Hide all except default constructor and data members from ROOT
#if USE_MODERN_FEATURES
    typedef detail::RawFragmentHeader::version_t     version_t;
    typedef detail::RawFragmentHeader::type_t        type_t;
    typedef detail::RawFragmentHeader::event_id_t    event_id_t;
    typedef detail::RawFragmentHeader::fragment_id_t fragment_id_t;

    static const version_t InvalidVersion  = detail::RawFragmentHeader::InvalidVersion;
    static const event_id_t InvalidEventID = detail::RawFragmentHeader::InvalidEventID;
    static const fragment_id_t InvalidFragmentID = detail::RawFragmentHeader::InvalidFragmentID;

    typedef std::vector<RawDataType>::reference      reference;
    typedef std::vector<RawDataType>::iterator       iterator;
    typedef std::vector<RawDataType>::const_iterator const_iterator;
    typedef std::vector<RawDataType>::value_type     value_type;


    // Create a Fragment ready to hold n words of payload, and with
    // all values zeroed.
    explicit Fragment(std::size_t n);

    // Create a fragment with the given event id and fragment id, and
    // with no data payload.
    Fragment(event_id_t eventID,
             fragment_id_t fragID,
             type_t type = type_t::DATA);

    // Print out summary information for this Fragment to the given stream.
    void print(std::ostream& os) const;

    size_t        size() const { return fragmentHeader()->word_count; }
    version_t     version() const { return fragmentHeader()->version; }
    type_t        type() const { return static_cast<type_t>(fragmentHeader()->type); }
    event_id_t    eventID() const { return fragmentHeader()->event_id; }
    fragment_id_t fragmentID() const { return fragmentHeader()->fragment_id; }

    // Return the number of words in the data payload. This does not
    // include the number of words in the header.
    std::size_t dataSize() const
    { return vals_.size() - detail::RawFragmentHeader::num_words(); }

    // Resize the data payload to hold sz words.
    void resize(std::size_t sz, RawDataType v = RawDataType())
    { vals_.resize(sz+detail::RawFragmentHeader::num_words(), v); }

    // Return an iterator to the beginning of the data payload.
    iterator dataBegin()
    { return vals_.begin() + detail::RawFragmentHeader::num_words(); }

    iterator dataEnd()
    { return vals_.end(); }

    const_iterator dataBegin() const
    { return vals_.begin() + detail::RawFragmentHeader::num_words(); }

    const_iterator dataEnd() const
    { return vals_.end(); }

    RawDataType& operator[](std::size_t i)
    { return vals_[i+detail::RawFragmentHeader::num_words()]; }

    RawDataType operator[](std::size_t i) const
    { return vals_[i+detail::RawFragmentHeader::num_words()]; }

    void clear()
    { vals_.erase(dataBegin(), dataEnd()); }

    bool empty()
    { return vals_.size() - detail::RawFragmentHeader::num_words() == 0; }

    void reserve(std::size_t cap)
    { vals_.reserve(cap + detail::RawFragmentHeader::num_words()); }

    void push_back(RawDataType v)
    { vals_.push_back(v); }

    void swap(Fragment& other)
    { vals_.swap(other.vals_); }

#endif

  private:
    std::vector<RawDataType> vals_;

#if USE_MODERN_FEATURES
    detail::RawFragmentHeader* fragmentHeader()
    { return reinterpret_cast<detail::RawFragmentHeader*>(&vals_[0]); }

    detail::RawFragmentHeader const* fragmentHeader() const
    { return reinterpret_cast<detail::RawFragmentHeader const*>(&vals_[0]); }
#endif
  };

#if USE_MODERN_FEATURES
  inline void swap(Fragment& x, Fragment& y) { x.swap(y); }

  inline
  std::ostream& operator<<(std::ostream& os, Fragment const& f)
  {
    f.print(os);
    return os;
  }
#endif

  typedef std::vector<uint64_t>           CompressedFragPart;
  typedef std::vector<CompressedFragPart> CompressedFragParts;

}
#endif /* artdaq_DAQdata_Fragment_hh */
