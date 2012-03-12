#include "artdaq/DAQdata/Fragment.hh"
#include <ostream>

namespace artdaq
{
  using detail::RawFragmentHeader;

  Fragment::Fragment() :
    vals_(RawFragmentHeader::num_words(), 0)
  { }

  Fragment::Fragment(std::size_t n) :
    vals_(n+RawFragmentHeader::num_words(), 0)
  {
    fragmentHeader()->word_count  = vals_.size();
    fragmentHeader()->type        = Fragment::InvalidType;
    fragmentHeader()->event_id    = Fragment::InvalidEventID;
    fragmentHeader()->fragment_id = Fragment::InvalidFragmentID;
  }

  Fragment::Fragment(event_id_t eventID,
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
  Fragment::print(std::ostream& os) const
  {
    os << " Fragment " << fragmentID()
       << ", WordCount " << size()
       << ", Event " << eventID()
       << '\n';
  }
#endif

}
