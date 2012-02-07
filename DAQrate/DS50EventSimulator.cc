#include "DS50EventSimulator.hh"
#include "fhiclcpp/ParameterSet.h"

namespace artdaq
{
  DS50EventSimulator::DS50EventSimulator(fhicl::ParameterSet const& ps) :
    events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
    events_gotten_(0)
  { }

  DS50EventSimulator::~DS50EventSimulator()
  { }

  bool
  DS50EventSimulator::getNext_(FragmentPtrs& frags)
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

}
