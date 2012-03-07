#ifndef DAQ_FRAGMENT_HH
#define DAQ_FRAGMENT_HH

#include <cstddef>
#include <iosfwd>
#include <vector>
#include <stdint.h>

#include "features.hh"

namespace artdaq
{
  typedef uint64_t RawDataType;

  // detail::RawFragmentHeader is an overlay that provides the user's
  // view of the data contained within a Fragment. It is intended to
  // be hidden from the user of Fragment, as an implementation
  // detail. The interface of Fragment is intended to be used to
  // access the data.

  namespace detail { struct RawFragmentHeader; }
  struct detail::RawFragmentHeader
  {
    // TODO: should Fragments know about Run and SubRun numbers?
    // If they do, then RawEventHeader should be updated.
    typedef uint16_t version_t;
    typedef uint16_t type_t;
    typedef uint64_t event_id_t;
    typedef uint16_t fragment_id_t;

    // Each of the following invalid values is chosen based on the
    // size of the bitfield in which the corresponding data are
    // encoded; if any of the sizes are changed, the corresponding
    // values must be updated.
    static const version_t InvalidVersion  = 0xFFFF;
    static const type_t    InvalidType     = 0xFFFF;
    static const event_id_t InvalidEventID = 0xFFFFFFFFFFFF;
    static const fragment_id_t InvalidFragmentID = 0xFFFF;

    RawDataType word_count  : 32; // number of RawDataTypes in this Fragment
    RawDataType version     : 16;
    RawDataType type        : 16;

    RawDataType event_id    : 48;
    RawDataType fragment_id : 16;

#if USE_MODERN_FEATURES
    constexpr static std::size_t num_words() 
    { return sizeof(detail::RawFragmentHeader); }
#endif
  };

  class Fragment
  {
  public:
    typedef detail::RawFragmentHeader::version_t     version_t;
    typedef detail::RawFragmentHeader::type_t        type_t;
    typedef detail::RawFragmentHeader::event_id_t    event_id_t;
    typedef detail::RawFragmentHeader::fragment_id_t fragment_id_t;

    static const version_t InvalidVersion  = detail::RawFragmentHeader::InvalidVersion;
    static const type_t    InvalidType     = detail::RawFragmentHeader::InvalidType;
    static const event_id_t InvalidEventID = detail::RawFragmentHeader::InvalidEventID;
    static const fragment_id_t InvalidFragmentID = detail::RawFragmentHeader::InvalidFragmentID;

    typedef std::vector<RawDataType>::reference      reference;
    typedef std::vector<RawDataType>::iterator       iterator;
    typedef std::vector<RawDataType>::const_iterator const_iterator;
    typedef std::vector<RawDataType>::value_type     value_type;

    // Create a Fragment with all header values zeroed.
    Fragment();

    // Create a Fragment ready to hold n words of payload, and with
    // all values zeroed.
    explicit Fragment(std::size_t n);

    // Create a fragment with the given event id and fragment id, and
    // with no data payload.
    Fragment(event_id_t eventID, fragment_id_t fragID, type_t type=0);

    // Print out summary information for this Fragment to the given stream.
    void print(std::ostream& os) const;

#if USE_MODERN_FEATURES
    size_t        size() const { return fragmentHeader()->word_count; }
    version_t     version() const { return fragmentHeader()->version; }
    type_t        type() const { return fragmentHeader()->type; }
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
#endif

  inline
  std::ostream& operator<<(std::ostream& os, Fragment const& f)
  {
    f.print(os);
    return os;
  }

  typedef std::vector<uint64_t>           CompressedFragPart;
  typedef std::vector<CompressedFragPart> CompressedFragParts;

}
#endif
