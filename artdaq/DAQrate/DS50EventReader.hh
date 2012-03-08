#ifndef artdaq_DAQrate_DS50EventReader_hh
#define artdaq_DAQrate_DS50EventReader_hh

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/DS50EventGenerator.hh"

namespace artdaq
{
  // DS50EventReader reads DS50 events from a file or set of files.

  class DS50EventReader : public DS50EventGenerator
  {
  public:
    explicit DS50EventReader(fhicl::ParameterSet const&);
    virtual ~DS50EventReader();

  private:
    virtual bool getNext_(FragmentPtrs& output);
  };
}

#endif /* artdaq_DAQrate_DS50EventReader_hh */
