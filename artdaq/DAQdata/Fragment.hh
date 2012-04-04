#ifndef artdaq_DAQdata_Fragment_hh
#define artdaq_DAQdata_Fragment_hh

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <vector>
#include <stdint.h>

#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"
#include "artdaq/DAQdata/features.hh"

namespace artdaq {
  typedef detail::RawFragmentHeader::RawDataType RawDataType;

  class Fragment;

  std::ostream & operator<<(std::ostream & os, Fragment const & f);
}

class artdaq::Fragment {
public:
  // Create a Fragment with all header values zeroed.
  Fragment();

  // Hide most things from ROOT.
#if USE_MODERN_FEATURES
  typedef detail::RawFragmentHeader::version_t     version_t;
  typedef detail::RawFragmentHeader::type_t        type_t;
  typedef detail::RawFragmentHeader::sequence_id_t    sequence_id_t;
  typedef detail::RawFragmentHeader::fragment_id_t fragment_id_t;

  static version_t const InvalidVersion;
  static sequence_id_t const InvalidSequenceID;
  static fragment_id_t const InvalidFragmentID;

  typedef std::vector<RawDataType>::reference       reference;
  typedef std::vector<RawDataType>::iterator        iterator;
  typedef std::vector<RawDataType>::const_iterator  const_iterator;
  typedef std::vector<RawDataType>::value_type      value_type;
  typedef std::vector<RawDataType>::difference_type difference_type;
  typedef std::vector<RawDataType>::size_type       size_type;

  // Create a Fragment ready to hold n words of payload, and with
  // all values zeroed.
  explicit Fragment(std::size_t n);

  // Create a fragment with the given event id and fragment id, and
  // with no data payload.
  Fragment(sequence_id_t sequenceID,
           fragment_id_t fragID,
           type_t type = type_t::DATA);

  // Print out summary information for this Fragment to the given stream.
  void print(std::ostream & os) const;

  // Header accessors
  std::size_t   size() const;
  version_t     version() const;
  type_t        type() const;
  sequence_id_t    sequenceID() const;
  fragment_id_t fragmentID() const;

  // Header setters
  void setVersion(version_t version);
  void setType(type_t type);
  void setSequenceID(sequence_id_t sequence_id);
  void setFragmentID(fragment_id_t fragment_id);

  // Return the number of words in the data payload. This does not
  // include the number of words in the header.
  std::size_t dataSize() const;

  // Resize the data payload to hold sz words.
  void resize(std::size_t sz, RawDataType v = RawDataType());

  // Return an iterator to the beginning of the data payload (post-header).
  iterator dataBegin();
  // ... and the end
  iterator dataEnd();

  // Return an iterator to the beginning of the header (should be used
  // for serialization only: use setters for preference).
  iterator headerBegin();

  const_iterator dataBegin() const;
  const_iterator dataEnd() const;
  const_iterator headerBegin() const; // See note for non-const, above.
  void clear();
  bool empty();
  void reserve(std::size_t cap);
  void swap(Fragment & other);

  static Fragment eodFrag(size_t nFragsToExpect);

  template <class InputIterator>
  static
  Fragment
  dataFrag(sequence_id_t sequenceID,
           fragment_id_t fragID,
           InputIterator i,
           InputIterator e);
#endif

private:
  void updateSize_();
  std::vector<RawDataType> vals_;

#if USE_MODERN_FEATURES
  detail::RawFragmentHeader * fragmentHeader();
  detail::RawFragmentHeader const * fragmentHeader() const;
#endif
};

#if USE_MODERN_FEATURES
inline
std::size_t
artdaq::Fragment::size() const
{
  return fragmentHeader()->word_count;
}

inline
artdaq::Fragment::version_t
artdaq::Fragment::version() const
{
  return fragmentHeader()->version;
}

inline
artdaq::Fragment::type_t
artdaq::Fragment::type() const
{
  return static_cast<type_t>(fragmentHeader()->type);
}

inline
artdaq::Fragment::sequence_id_t
artdaq::Fragment::sequenceID() const
{
  return fragmentHeader()->sequence_id;
}

inline
artdaq::Fragment::fragment_id_t
artdaq::Fragment::fragmentID() const
{
  return fragmentHeader()->fragment_id;
}

inline
void
artdaq::Fragment::setVersion(version_t version)
{
  fragmentHeader()->version = version;
}

inline
void
artdaq::Fragment::setType(type_t type)
{
  fragmentHeader()->type = static_cast<uint8_t>(type);
}

inline
void
artdaq::Fragment::setSequenceID(sequence_id_t sequence_id)
{
  assert(sequence_id < 0x1000000000000);
  fragmentHeader()->sequence_id = sequence_id;
}

inline
void
artdaq::Fragment::setFragmentID(fragment_id_t fragment_id)
{
  fragmentHeader()->fragment_id = fragment_id;
}

inline
void
artdaq::Fragment::updateSize_()
{
  assert(vals_.size() < 0x100000000);
  fragmentHeader()->word_count = vals_.size();
}

inline
std::size_t
artdaq::Fragment::dataSize() const
{
  return vals_.size() - detail::RawFragmentHeader::num_words();
}

inline
void
artdaq::Fragment::resize(std::size_t sz, RawDataType v)
{
  vals_.resize(sz + detail::RawFragmentHeader::num_words(), v);
  updateSize_();
}

inline
artdaq::Fragment::iterator
artdaq::Fragment::dataBegin()
{
  return vals_.begin() + detail::RawFragmentHeader::num_words();
}

inline
artdaq::Fragment::iterator
artdaq::Fragment::dataEnd()
{
  return vals_.end();
}

inline
artdaq::Fragment::iterator
artdaq::Fragment::headerBegin()
{
  return vals_.begin();
}

inline
artdaq::Fragment::const_iterator
artdaq::Fragment::dataBegin() const
{
  return vals_.begin() + detail::RawFragmentHeader::num_words();
}

inline
artdaq::Fragment::const_iterator
artdaq::Fragment::dataEnd() const
{
  return vals_.end();
}

inline
artdaq::Fragment::const_iterator
artdaq::Fragment::headerBegin() const
{
  return vals_.begin();
}

inline
void
artdaq::Fragment::clear()
{
  vals_.erase(dataBegin(), dataEnd());
  updateSize_();
}

inline
bool
artdaq::Fragment::empty()
{
  return vals_.size() - detail::RawFragmentHeader::num_words() == 0;
}

inline
void
artdaq::Fragment::reserve(std::size_t cap)
{
  vals_.reserve(cap + detail::RawFragmentHeader::num_words());
}

inline
void
artdaq::Fragment::swap(Fragment & other)
{
  vals_.swap(other.vals_);
}

template <class InputIterator>
artdaq::Fragment
artdaq::Fragment::
dataFrag(sequence_id_t sequenceID,
         fragment_id_t fragID,
         InputIterator i,
         InputIterator e)
{
  Fragment result(sequenceID, fragID);
  result.vals_.reserve(std::distance(i, e) + detail::RawFragmentHeader::num_words());
  std::copy(i, e, std::back_inserter(result.vals_));
  return result;
}

inline
artdaq::detail::RawFragmentHeader *
artdaq::Fragment::fragmentHeader()
{
  return reinterpret_cast<detail::RawFragmentHeader *>(&vals_[0]);
}

inline
artdaq::detail::RawFragmentHeader const *
artdaq::Fragment::fragmentHeader() const
{
  return reinterpret_cast<detail::RawFragmentHeader const *>(&vals_[0]);
}

inline
void
swap(artdaq::Fragment & x, artdaq::Fragment & y)
{
  x.swap(y);
}

inline
std::ostream &
artdaq::operator<<(std::ostream & os, artdaq::Fragment const & f)
{
  f.print(os);
  return os;
}
#endif/* USE_MODERN_FEATURES */

#endif /* artdaq_DAQdata_Fragment_hh */
