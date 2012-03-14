#ifndef artdaq_DAQrate_DS50FragmentReader_hh
#define artdaq_DAQrate_DS50FragmentReader_hh

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/FragmentGenerator.hh"

#include <string>
#include <vector>

namespace artdaq {
  // DS50FragmentReader reads DS50 events from a file or set of files.

  class DS50FragmentReader : public FragmentGenerator {
  public:
    explicit DS50FragmentReader(fhicl::ParameterSet const &);
    virtual ~DS50FragmentReader();

  private:
    virtual bool getNext_(FragmentPtrs & output);

    // Configuration.
    std::vector<std::string> const fileNames_;
    uint64_t const max_set_size_bytes_;

    // State
    std::pair<std::vector<std::string>::const_iterator, uint64_t> next_point_;

  };
}

#endif /* artdaq_DAQrate_DS50FragmentReader_hh */
