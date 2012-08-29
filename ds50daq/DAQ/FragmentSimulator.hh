#ifndef ds50daq_DAQ_FragmentSimulator_hh
#define ds50daq_DAQ_FragmentSimulator_hh

#include "fhiclcpp/fwd.h"
#include "ds50daq/DAQ/DS50Types.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"

#include <random>
#include <vector>

namespace ds50 {
  class FragmentSimulator;
}

// FragmentSimulator creates simulated DS50 events, with data
// distributed according to a "histogram" provided in the configuration
// data.

class ds50::FragmentSimulator : public artdaq::FragmentGenerator {
public:
  explicit FragmentSimulator(fhicl::ParameterSet const & ps);

private:
  virtual bool getNext_(artdaq::FragmentPtrs & output);

  std::size_t current_event_num_;
  std::size_t const events_to_generate_; // go forever if this is 0
  std::size_t const fragments_per_event_;
  std::size_t const starting_fragment_id_;
  std::size_t const nChannels_;
  artdaq::RawDataType const run_number_;
  std::mt19937 engine_;
  std::vector<std::vector<size_t>> adc_freqs_;
  std::vector<std::discrete_distribution<adc_type>> content_generator_;
};

#endif /* ds50daq_DAQ_FragmentSimulator_hh */
