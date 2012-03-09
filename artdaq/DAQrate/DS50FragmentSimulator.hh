#ifndef artdaq_DAQrate_DS50FragmentSimulator_hh
#define artdaq_DAQrate_DS50FragmentSimulator_hh

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/FragmentGenerator.hh"

namespace artdaq
{

  // DS50FragmentSimulator creates simulated DS50 events, with data
  // distributed according to a "histogram" provided in the
  // configuration data.

  class DS50FragmentSimulator : public FragmentGenerator
  {
  public:
    explicit DS50FragmentSimulator(fhicl::ParameterSet const& ps);
    virtual ~DS50FragmentSimulator();

  private:
    virtual bool getNext_(FragmentPtrs& output);

    std::size_t events_to_generate_; // go forever if this is 0
    std::size_t events_gotten_;
    RawDataType run_number_;
  };
}

#endif /* artdaq_DAQrate_DS50FragmentSimulator_hh */
