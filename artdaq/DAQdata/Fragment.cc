#include "artdaq/DAQdata/Fragment.hh"

#include <cmath>
#include <iostream>

using artdaq::detail::RawFragmentHeader;

artdaq::Fragment::version_t const artdaq::Fragment::InvalidVersion =
  detail::RawFragmentHeader::InvalidVersion;
artdaq::Fragment::sequence_id_t const artdaq::Fragment::InvalidSequenceID =
  detail::RawFragmentHeader::InvalidSequenceID;
artdaq::Fragment::fragment_id_t const artdaq::Fragment::InvalidFragmentID =
  detail::RawFragmentHeader::InvalidFragmentID;

artdaq::Fragment::Fragment() :
  vals_(RawFragmentHeader::num_words(), 0)
{
  updateSize_();
}

artdaq::Fragment::Fragment(std::size_t n) :
  vals_(n + RawFragmentHeader::num_words(), 0)
{
  updateSize_();
  fragmentHeader()->type        = type_t::INVALID;
  fragmentHeader()->sequence_id    = Fragment::InvalidSequenceID;
  fragmentHeader()->fragment_id = Fragment::InvalidFragmentID;
}

artdaq::Fragment::Fragment(sequence_id_t sequenceID,
                           fragment_id_t fragID,
                           type_t type) :
  vals_(RawFragmentHeader::num_words(), 0)
{
  updateSize_();
  fragmentHeader()->type        = type;
  fragmentHeader()->sequence_id    = sequenceID;
  fragmentHeader()->fragment_id = fragID;
}

#if USE_MODERN_FEATURES
void
artdaq::Fragment::print(std::ostream & os) const
{
  os << " Fragment " << fragmentID()
     << ", WordCount " << size()
     << ", Event " << sequenceID()
     << '\n';
}

artdaq::Fragment
artdaq::Fragment::eodFrag(size_t nFragsToExpect)
{
  Fragment result(static_cast<size_t>(ceil(sizeof(nFragsToExpect) /
                                      static_cast<double>(sizeof(value_type)))));
  result.setType(Fragment::type_t::END_OF_DATA);
  *result.dataBegin() = nFragsToExpect;
  return result;
}
#endif
