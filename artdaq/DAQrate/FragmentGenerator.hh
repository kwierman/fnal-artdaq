#ifndef artdaq_DAQrate_FragmentGenerator_hh
#define artdaq_DAQrate_FragmentGenerator_hh

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"

namespace artdaq
{
  // FragmentGenerator defines the abstract interface for obtaining
  // DS50 events. Subclass implement 'getNext' by various means, such
  // as reading from files or random generation.

  class FragmentGenerator
  {
  public:
    virtual ~FragmentGenerator();
    // Obtain the next collection of Fragments. Return false to
    // indicate end-of-data.
    bool getNext(FragmentPtrs& output);

  private:
    virtual bool getNext_(FragmentPtrs& output) = 0;
  };
}

#endif /* artdaq_DAQrate_FragmentGenerator_hh */
