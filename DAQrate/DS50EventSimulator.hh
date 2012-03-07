#ifndef artdaq_DAQrate_DS50EventSimulator_hh
#define artdaq_DAQrate_DS50EventSimulator_hh

#include "fhiclcpp/fwd.h"
#include "DAQdata/Fragments.hh"
#include "DS50EventGenerator.hh"

namespace artdaq
{

  // DS50EventSimulator creates simulated DS50 events, with data
  // distributed according to a "histogram" provided in the
  // configuration data.

  class DS50EventSimulator : public DS50EventGenerator
  {
  public:
    explicit DS50EventSimulator(fhicl::ParameterSet const& ps);
    virtual ~DS50EventSimulator();

  private:
    virtual bool getNext_(FragmentPtrs& output);

    std::size_t events_to_generate_; // go forever if this is 0
    std::size_t events_gotten_;
    RawDataType run_number_;
  };
}

#endif
