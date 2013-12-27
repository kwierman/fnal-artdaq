#ifndef artdaq_DAQdata_GeneratorMacros_hh
#define artdaq_DAQdata_GeneratorMacros_hh

#include "artdaq/DAQdata/FragmentGenerator.hh"
#include "fhiclcpp/fwd.h"

#include <memory>

namespace artdaq {
  typedef std::unique_ptr<artdaq::FragmentGenerator>
  (makeFunc_t) (fhicl::ParameterSet const & ps);
}

#define DEFINE_ARTDAQ_GENERATOR(klass)                                \
  extern "C"                                                          \
  std::unique_ptr<artdaq::FragmentGenerator>                          \
  make(fhicl::ParameterSet const & ps) {                              \
    return std::unique_ptr<artdaq::FragmentGenerator>(new klass(ps)); \
  }

#endif /* artdaq_DAQdata_GeneratorMacros_hh */
