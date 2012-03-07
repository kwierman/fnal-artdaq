#ifndef artdaq_DAQrate_DS50EventGenerator_hh
#define artdaq_DAQrate_DS50EventGenerator_hh

#include "fhiclcpp/fwd.h"
#include "DAQdata/Fragments.hh"

namespace artdaq
{
  // DS50EventGenerator defines the abstract interface for obtaining
  // DS50 events. Subclass implement 'getNext' by various means, such
  // as reading from files or random generation.

  class DS50EventGenerator
  {
  public:
    virtual ~DS50EventGenerator();
    // Obtain the next collection of Fragments. Return false to
    // indicate end-of-data.
    bool getNext(FragmentPtrs& output);

    static constexpr std::size_t num_boards() { return 5; }

  private:
    virtual bool getNext_(FragmentPtrs& output) = 0;
  };
}

#endif
