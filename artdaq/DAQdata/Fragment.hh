#ifndef artdaq_DAQdata_Fragment_hh
#define artdaq_DAQdata_Fragment_hh

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <vector>
#include <memory>
#include <stdint.h>
#include <string.h>

#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"
#include "artdaq/DAQdata/features.hh"

namespace artdaq {
  typedef detail::RawFragmentHeader::RawDataType RawDataType;

  class Fragment;
  bool fragmentSequenceIDCompare(Fragment i, Fragment j);

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
  typedef detail::RawFragmentHeader::sequence_id_t sequence_id_t;
  typedef detail::RawFragmentHeader::fragment_id_t fragment_id_t;

  static constexpr version_t InvalidVersion =
    detail::RawFragmentHeader::InvalidVersion;
  static constexpr sequence_id_t InvalidSequenceID =
    detail::RawFragmentHeader::InvalidSequenceID;
  static constexpr fragment_id_t InvalidFragmentID =
    detail::RawFragmentHeader::InvalidFragmentID;

  static constexpr type_t InvalidFragmentType =
    detail::RawFragmentHeader::InvalidFragmentType;
  static constexpr type_t EndOfDataFragmentType =
    detail::RawFragmentHeader::EndOfDataFragmentType;
  static constexpr type_t DataFragmentType =
    detail::RawFragmentHeader::DataFragmentType;
  static constexpr type_t InitFragmentType =
    detail::RawFragmentHeader::InitFragmentType;
  static constexpr type_t EndOfRunFragmentType =
    detail::RawFragmentHeader::EndOfRunFragmentType;
  static constexpr type_t EndOfSubrunFragmentType =
    detail::RawFragmentHeader::EndOfSubrunFragmentType;
  static constexpr type_t ShutdownFragmentType =
    detail::RawFragmentHeader::ShutdownFragmentType;
  static constexpr type_t FirstUserFragmentType =
    detail::RawFragmentHeader::FIRST_USER_TYPE;

  static constexpr bool isUserFragmentType(type_t fragmentType);
  static constexpr bool isSystemFragmentType(type_t fragmentType);

  typedef std::vector<RawDataType>::reference       reference;
  typedef std::vector<RawDataType>::iterator        iterator;
  typedef std::vector<RawDataType>::const_iterator  const_iterator;
  typedef std::vector<RawDataType>::value_type      value_type;
  typedef std::vector<RawDataType>::difference_type difference_type;
  typedef std::vector<RawDataType>::size_type       size_type;

  // Create a Fragment ready to hold n words of payload, and with
  // all values zeroed.
  explicit Fragment(std::size_t n);

  // Create a Fragment ready to hold the specified number of words
  // of payload, with the specified sequence ID, fragment ID,
  // fragment type, and metadata.
  template <class T>
  Fragment(std::size_t payload_size, sequence_id_t sequence_id,
           fragment_id_t fragment_id, type_t type, const T & metadata);

  // Create a fragment with the given event id and fragment id, and
  // with no data payload.
  Fragment(sequence_id_t sequenceID,
           fragment_id_t fragID,
           type_t type = Fragment::DataFragmentType);

  // Print out summary information for this Fragment to the given stream.
  void print(std::ostream & os) const;

  // Header accessors
  std::size_t   size() const;
  version_t     version() const;
  type_t        type() const;
  sequence_id_t sequenceID() const;
  fragment_id_t fragmentID() const;

  // Header setters
  void setVersion(version_t version);
  void setUserType(type_t type);
  void setSystemType(type_t type);
  void setSequenceID(sequence_id_t sequence_id);
  void setFragmentID(fragment_id_t fragment_id);

  // Return the number of words in the data payload. This does not
  // include the number of words in the header.
  std::size_t dataSize() const;

  // Test whether this Fragment has metadata
  bool hasMetadata() const;

  // Return a pointer to the metadata. This throws an exception
  // if the fragment contains no metadata.
  template <class T> T * metadata();
  template <class T> T const * metadata() const;

  // Set the metadata in the Fragment to the contents of
  // the specified structure.  This throws an exception if
  // the Fragment already contains metadata.
  template <class T> void setMetadata(const T & md);

  // Resize the data payload to hold sz words.
  void resize(std::size_t sz, RawDataType v = RawDataType());

  // Resize the fragment to hold the number of words in the header.
  void autoResize();

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

  RawDataType * dataAddress();
  RawDataType * metadataAddress();   // for internal use only
  RawDataType * headerAddress();

  static std::unique_ptr<Fragment> eodFrag(size_t nFragsToExpect);

  // 12-Apr-2013, KAB - this method is deprecated, please do not use
  template <class InputIterator>
  static
  Fragment
  dataFrag(sequence_id_t sequenceID,
           fragment_id_t fragID,
           InputIterator i,
           InputIterator e);

  static
  Fragment
  dataFrag(sequence_id_t sequenceID,
           fragment_id_t fragID,
           RawDataType const * dataPtr,
           size_t dataSize);
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
bool
constexpr
artdaq::Fragment::
isUserFragmentType(type_t fragmentType)
{
  return fragmentType >= detail::RawFragmentHeader::FIRST_USER_TYPE &&
    fragmentType <= detail::RawFragmentHeader::LAST_USER_TYPE;
}

inline
bool
constexpr
artdaq::Fragment::
isSystemFragmentType(type_t fragmentType)
{
  return fragmentType >= detail::RawFragmentHeader::FIRST_SYSTEM_TYPE;
}

template <class T>
artdaq::Fragment::
Fragment(std::size_t payload_size, sequence_id_t sequence_id,
         fragment_id_t fragment_id, type_t type, const T & metadata) :
  vals_(payload_size + artdaq::detail::RawFragmentHeader::num_words() +
        std::ceil(sizeof(T)/sizeof(artdaq::RawDataType)), 0)
{
  updateSize_();
  fragmentHeader()->sequence_id = sequence_id;
  fragmentHeader()->fragment_id = fragment_id;
  fragmentHeader()->type        = type;

  size_t max_md_wc =
    (256 * sizeof(detail::RawFragmentHeader::metadata_word_count_t)) - 1;
  size_t requested_md_wc = std::ceil(sizeof(T)/sizeof(artdaq::RawDataType));
  if (requested_md_wc > max_md_wc) {
    throw cet::exception("InvalidRequest")
      << "The requested metadata structure is too large: "
      << "requested word count = " << requested_md_wc
      << ", maximum word count = " << max_md_wc;
  }
  fragmentHeader()->metadata_word_count = requested_md_wc;

  memcpy(metadataAddress(), &metadata, sizeof(T));
}

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
artdaq::Fragment::setUserType(type_t type)
{
  fragmentHeader()->setUserType(static_cast<uint8_t>(type));
}

inline
void
artdaq::Fragment::setSystemType(type_t type)
{
  fragmentHeader()->setSystemType(static_cast<uint8_t>(type));
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
  return vals_.size() - detail::RawFragmentHeader::num_words() -
    fragmentHeader()->metadata_word_count;
}

inline
bool
artdaq::Fragment::hasMetadata() const
{
  return fragmentHeader()->metadata_word_count > 0;
}

template <class T>
T *
artdaq::Fragment::metadata()
{
  if (fragmentHeader()->metadata_word_count < 1) {
    throw cet::exception("InvalidRequest")
      << "No metadata has been stored in this Fragment.";
  }
  return reinterpret_cast<T *>
    (&vals_[detail::RawFragmentHeader::num_words()]);
}

template <class T>
T const *
artdaq::Fragment::metadata() const
{
  if (fragmentHeader()->metadata_word_count < 1) {
    throw cet::exception("InvalidRequest")
      << "No metadata has been stored in this Fragment.";
  }
  return reinterpret_cast<T const *>
    (&vals_[detail::RawFragmentHeader::num_words()]);
}

template <class T>
void
artdaq::Fragment::setMetadata(const T & metadata)
{
  if (fragmentHeader()->metadata_word_count > 0) {
    throw cet::exception("InvalidRequest")
      << "Metadata has already been stored in this Fragment.";
  }

  size_t max_md_wc =
    (256 * sizeof(detail::RawFragmentHeader::metadata_word_count_t)) - 1;
  size_t requested_md_wc = std::ceil(sizeof(T)/sizeof(artdaq::RawDataType));
  if (requested_md_wc > max_md_wc) {
    throw cet::exception("InvalidRequest")
      << "The requested metadata structure is too large: "
      << "requested word count = " << requested_md_wc
      << ", maximum word count = " << max_md_wc;
  }

  vals_.insert(vals_.begin()+detail::RawFragmentHeader::num_words(),
               requested_md_wc, 0);
  updateSize_();
  fragmentHeader()->metadata_word_count = requested_md_wc;

  memcpy(metadataAddress(), &metadata, sizeof(T));
}

inline
void
artdaq::Fragment::resize(std::size_t sz, RawDataType v)
{
  vals_.resize(sz + fragmentHeader()->metadata_word_count +
               detail::RawFragmentHeader::num_words(), v);
  updateSize_();
}

inline
void
artdaq::Fragment::autoResize()
{
  vals_.resize(fragmentHeader()->word_count);
  updateSize_();
}

inline
artdaq::Fragment::iterator
artdaq::Fragment::dataBegin()
{
  return vals_.begin() + detail::RawFragmentHeader::num_words() +
    fragmentHeader()->metadata_word_count;
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
  return vals_.begin() + detail::RawFragmentHeader::num_words() +
    fragmentHeader()->metadata_word_count;
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
  return (vals_.size() - detail::RawFragmentHeader::num_words() -
          fragmentHeader()->metadata_word_count) == 0;
}

inline
void
artdaq::Fragment::reserve(std::size_t cap)
{
  vals_.reserve(cap + detail::RawFragmentHeader::num_words() +
                fragmentHeader()->metadata_word_count);
}

inline
void
artdaq::Fragment::swap(Fragment & other)
{
  vals_.swap(other.vals_);
}

inline
artdaq::RawDataType *
artdaq::Fragment::dataAddress()
{
  return &vals_[0] + detail::RawFragmentHeader::num_words() +
    fragmentHeader()->metadata_word_count;
}

inline
artdaq::RawDataType *
artdaq::Fragment::metadataAddress()
{
  if (fragmentHeader()->metadata_word_count < 1) {
    throw cet::exception("InvalidRequest")
      << "No metadata has been stored in this Fragment.";
  }
  return &vals_[0] + detail::RawFragmentHeader::num_words();
}

inline
artdaq::RawDataType *
artdaq::Fragment::headerAddress()
{
  return &vals_[0];
}

// 12-Apr-2013, KAB - this method is deprecated, please do not use
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

