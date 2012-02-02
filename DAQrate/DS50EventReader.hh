#ifndef artdaq_DAQrate_DS50EventREader_hh
#define artdaq_DAQrate_DS50EventREader_hh

#include "fhiclcpp/fwd.h"
#include "DAQdata/Fragments.hh"

namespace artdaq
{
  class DS50EventReader
  {
  public:
    explicit DS50EventReader(fhicl::ParameterSet const& ps);

    // Obtain the next collection of Fragments. Return false to
    // indicate end-of-data.
    bool getNext(FragmentPtrs& output);
  private:
    bool        do_random_;        // generate random data if true
    std::size_t events_to_generate_; // go forever if this is 0
    std::size_t events_gotten_;

    // Obtain the next set of fragments through random generation.
    bool getNext_random_(FragmentPtrs& output);

    // Obtain the next set of fragments through reading the input
    // file.
    bool getNext_read_(FragmentPtrs& output);
  };
}

#endif
