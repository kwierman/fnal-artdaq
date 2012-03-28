#include "artdaq/DAQdata/RawEvent.hh"
#include <ostream>


namespace artdaq
{
  
  void RawEvent::print(std::ostream& os) const
  {
    os << "Run " << runID()
       << ", Subrun " << subrunID()
       << ", Event " << sequenceID()
       << ", FragCount " << numFragments()
       << ", WordCount " << wordCount()
       << '\n';

    for (auto const& frag : fragments_) os << *frag << '\n';
  }

}
