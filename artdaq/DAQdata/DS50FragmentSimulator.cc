#include "artdaq/DAQdata/DS50FragmentSimulator.hh"
#include "fhiclcpp/ParameterSet.h"

using namespace artdaq;

ds50::FragmentSimulator::FragmentSimulator(fhicl::ParameterSet const & ps) :
  current_event_num_(0),
  events_to_generate_(ps.get<size_t>("events_to_generate", 0)),
  fragments_per_event_(ps.get<size_t>("fragments_per_event", 5)),
  run_number_(ps.get<RawDataType>("run_number"))
{ }

ds50::FragmentSimulator::~FragmentSimulator()
{ }

bool
ds50::FragmentSimulator::getNext_(FragmentPtrs & frags)
{
  ++current_event_num_;
  if (events_to_generate_ != 0 &&
      current_event_num_ > events_to_generate_) {
    return false;
  }
  RawDataType fragID(0);
  for (size_t i = 0; i < fragments_per_event_; ++i) {
    ++fragID;
    frags.emplace_back(new Fragment(current_event_num_, fragID));
  }
  return true;
}
