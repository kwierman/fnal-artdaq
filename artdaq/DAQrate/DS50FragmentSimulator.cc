#include "artdaq/DAQrate/DS50FragmentSimulator.hh"
#include "fhiclcpp/ParameterSet.h"

artdaq::DS50FragmentSimulator::DS50FragmentSimulator(fhicl::ParameterSet const & ps) :
  events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
  events_gotten_(0),
  fragments_per_event_(ps.get<size_t>("fragments_per_event", 5)),
  run_number_(ps.get<RawDataType>("run_number"))
{ }

artdaq::DS50FragmentSimulator::~DS50FragmentSimulator()
{ }

bool
artdaq::DS50FragmentSimulator::getNext_(FragmentPtrs & frags)
{
  ++events_gotten_;
  if (events_to_generate_ != 0 &&
      events_gotten_ > events_to_generate_) {
    return false;
  }
  FragmentPtrs tmp;
  RawDataType fragID(0);
  for (size_t i = 0; i < fragments_per_event_; ++i) {
    ++fragID;
    frags.emplace_back(new Fragment(events_gotten_, fragID));
  }
  return true;
}
