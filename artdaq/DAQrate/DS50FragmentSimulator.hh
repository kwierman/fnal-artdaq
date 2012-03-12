#ifndef artdaq_DAQrate_DS50FragmentSimulator_hh
#define artdaq_DAQrate_DS50FragmentSimulator_hh

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/FragmentGenerator.hh"

namespace artdaq {
  class DS50FragmentSimulator;
}

// DS50FragmentSimulator creates simulated DS50 events, with data
// distributed according to a "histogram" provided in the configuration
// data.

class artdaq::DS50FragmentSimulator : public artdaq::FragmentGenerator {
public:
  explicit DS50FragmentSimulator(fhicl::ParameterSet const & ps);
  virtual ~DS50FragmentSimulator();

private:
  virtual bool getNext_(FragmentPtrs & output);

  std::size_t current_event_num_;
  std::size_t const events_to_generate_; // go forever if this is 0
  std::size_t const fragments_per_event_;
  RawDataType const run_number_;
};

#endif /* artdaq_DAQrate_DS50FragmentSimulator_hh */
