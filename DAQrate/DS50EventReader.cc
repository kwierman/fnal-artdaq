#include "DS50EventReader.hh"
#include <string>

#include "fhiclcpp/ParameterSet.h"

using fhicl::ParameterSet;

const std::size_t NUM_BOARDS = 5; // Should this be a configuration parameter?

namespace artdaq
{
  DS50EventReader::DS50EventReader(ParameterSet const& ps) :
    do_random_(ps.get<bool>("do_random", false)),
    events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
    events_gotten_(0)
  {
    if (!do_random_) 
      throw std::string("Only random generation supported by DS50Reader.");
  }

  bool
  DS50EventReader::getNext(FragmentPtrs& output)
  {
    bool rc = (do_random_ ? getNext_random_(output) : getNext_read_(output));
    if (rc) ++events_gotten_;
    return rc;
  }

  bool
  DS50EventReader::getNext_random_(FragmentPtrs& frags)
  {
    if (events_to_generate_ != 0 && events_to_generate_ <= events_gotten_)
      return false;
    FragmentPtrs tmp(5);
    for (auto& pfrag : tmp) {
      pfrag.reset(new Fragment());
    }
    frags.swap(tmp);
    return true;
  }

  bool
  DS50EventReader::getNext_read_(FragmentPtrs&)
  {
    return false;
  }

}
