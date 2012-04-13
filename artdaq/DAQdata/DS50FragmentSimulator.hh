#ifndef artdaq_DAQdata_DS50FragmentSimulator_hh
#define artdaq_DAQdata_DS50FragmentSimulator_hh

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"

namespace ds50 {
  class FragmentSimulator;
}

// DS50FragmentSimulator creates simulated DS50 events, with data
// distributed according to a "histogram" provided in the configuration
// data.

class ds50::FragmentSimulator : public artdaq::FragmentGenerator {
public:
  explicit FragmentSimulator(fhicl::ParameterSet const & ps);
  virtual ~FragmentSimulator();

private:
  virtual bool getNext_(artdaq::FragmentPtrs & output);

  std::size_t current_event_num_;
  std::size_t const events_to_generate_; // go forever if this is 0
  std::size_t const fragments_per_event_;
  artdaq::RawDataType const run_number_;
};

#endif /* artdaq_DAQdata_DS50FragmentSimulator_hh */
