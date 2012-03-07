#include "DS50EventSimulator.hh"
#include "fhiclcpp/ParameterSet.h"

namespace artdaq
{
  DS50EventSimulator::DS50EventSimulator(fhicl::ParameterSet const& ps) :
    events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
    events_gotten_(0),
    run_number_(ps.get<RawDataType>("run_number"))
  { }

  DS50EventSimulator::~DS50EventSimulator()
  { }

  bool
  DS50EventSimulator::getNext_(FragmentPtrs& frags)
  {
    ++events_gotten_;
    if (events_to_generate_ != 0 && events_to_generate_ <= events_gotten_)
      return false;
    FragmentPtrs tmp(num_boards());
    RawDataType fragID(0);
    for (auto& pfrag : tmp) {
      ++fragID;
      pfrag.reset(new Fragment(events_gotten_, fragID));
    }
    frags.swap(tmp);
    return true;
  }

}
