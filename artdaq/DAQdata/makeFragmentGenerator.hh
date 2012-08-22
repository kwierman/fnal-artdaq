#ifndef artdaq_DAQdata_makeFragmentGenerator_hh
#define artdaq_DAQdata_makeFragmentGenerator_hh
// Using LibraryManager, find the correct library and return an instance
// of the specified generator.

#include "fhiclcpp/fwd.h"

#include <memory>
#include <string>

namespace artdaq {

  class FragmentGenerator;

  std::unique_ptr<FragmentGenerator>
    makeFragmentGenerator(std::string const & generator_plugin_spec,
                          fhicl::ParameterSet const & ps);
}
#endif /* artdaq_DAQdata_makeFragmentGenerator_hh */
