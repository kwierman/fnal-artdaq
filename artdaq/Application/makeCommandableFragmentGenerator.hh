#ifndef artdaq_Application_makeCommandableFragmentGenerator_hh
#define artdaq_Application_makeCommandableFragmentGenerator_hh
// Using LibraryManager, find the correct library and return an instance
// of the specified generator.

#include "fhiclcpp/fwd.h"

#include <memory>
#include <string>

namespace artdaq {

  class CommandableFragmentGenerator;

  std::unique_ptr<CommandableFragmentGenerator>
    makeCommandableFragmentGenerator(std::string const & generator_plugin_spec,
                          fhicl::ParameterSet const & ps);
}
#endif /* artdaq_Application_makeCommandableFragmentGenerator_hh */
