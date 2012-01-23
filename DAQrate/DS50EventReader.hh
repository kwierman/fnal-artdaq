#ifndef artdaq_DAQrate_DS50EventREader_hh
#define artdaq_DAQrate_DS50EventREader_hh

#include "fhiclcpp/ParameterSet.h"

namespace artdaq
{
  class DS50EventReader
  {
  public:
    explicit DS50EventReader(fhicl::ParameterSet const& ps);
  private:
    bool do_random_;
  };
}

#endif
