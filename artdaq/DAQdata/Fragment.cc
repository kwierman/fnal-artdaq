#include "artdaq/DAQdata/Fragment.hh"

// See implementation of Streamer() below.
#ifdef ARTDAQ_WANT_FRAGMENT_STREAMER
#include "TBuffer.h"
#endif

#include <iostream>

using artdaq::detail::RawFragmentHeader;

artdaq::Fragment::version_t const artdaq::Fragment::InvalidVersion =
  detail::RawFragmentHeader::InvalidVersion;
artdaq::Fragment::event_id_t const artdaq::Fragment::InvalidEventID =
  detail::RawFragmentHeader::InvalidEventID;
artdaq::Fragment::fragment_id_t const artdaq::Fragment::InvalidFragmentID =
  detail::RawFragmentHeader::InvalidFragmentID;

artdaq::Fragment::Fragment() :
  vals_(RawFragmentHeader::num_words(), 0)
{ }

artdaq::Fragment::Fragment(std::size_t n) :
  vals_(n + RawFragmentHeader::num_words(), 0)
{
  fragmentHeader()->word_count  = vals_.size();
  fragmentHeader()->type        = type_t::INVALID;
  fragmentHeader()->event_id    = Fragment::InvalidEventID;
  fragmentHeader()->fragment_id = Fragment::InvalidFragmentID;
}

artdaq::Fragment::Fragment(event_id_t eventID,
                           fragment_id_t fragID,
                           type_t type) :
  vals_(RawFragmentHeader::num_words(), 0)
{
  fragmentHeader()->word_count  = vals_.size();
  fragmentHeader()->type        = type;
  fragmentHeader()->event_id    = eventID;
  fragmentHeader()->fragment_id = fragID;
}

#if USE_MODERN_FEATURES
void
artdaq::Fragment::print(std::ostream & os) const
{
  os << " Fragment " << fragmentID()
     << ", WordCount " << size()
     << ", Event " << eventID()
     << '\n';
}
#endif

// Activate this code if/when we care about putting Fragments in ROOT
// files and retrieving them -- deactivated for now due to increase in
// link dependencies.
#ifdef ARTDAQ_WANT_FRAGMENT_STREAMER
void
artdaq::Fragment::Streamer(TBuffer & buf)
{
  if (buf.IsReading()) {
    if (vals_.size() != size()) {
      std::cerr << "WARNING: Fragment header word_count is inconsistent "
                << "on file: Fixing.\n"
                << *this;
      updateSize();
    }
  }
  else {
    updateSize();
  }
}
#endif
