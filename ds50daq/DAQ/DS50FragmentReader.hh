#ifndef artdaq_DAQdata_DS50FragmentReader_hh
#define artdaq_DAQdata_DS50FragmentReader_hh

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"

#include <string>
#include <vector>

namespace ds50 {
  // FragmentReader reads DS50 events from a file or set of files.
  class FragmentReader;
}

class ds50::FragmentReader : public artdaq::FragmentGenerator {
public:
  explicit FragmentReader(fhicl::ParameterSet const &);

private:
  virtual bool getNext_(artdaq::FragmentPtrs & output);

  // Configuration.
  std::vector<std::string> const fileNames_;
  uint64_t const max_set_size_bytes_;

  // State
  std::pair<std::vector<std::string>::const_iterator, uint64_t> next_point_;

};

#endif /* artdaq_DAQdata_DS50FragmentReader_hh */
