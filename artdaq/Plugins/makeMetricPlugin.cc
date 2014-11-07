#include "artdaq/Plugins/makeMetricPlugin.hh"

#include "artdaq/Plugins/MetricMacros.hh"
#include "fhiclcpp/ParameterSet.h"
#include "cetlib/BasicPluginFactory.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

std::unique_ptr<artdaq::MetricPlugin>
artdaq::makeMetricPlugin(std::string const & generator_plugin_spec,
                              fhicl::ParameterSet const & ps)
{
  static cet::BasicPluginFactory bpf("metric", "make");
  
  mf::LogInfo("MetricPluginFactory") << "Constructing metric plugin with type: \""
                                     << generator_plugin_spec << "\".";

  return bpf.makePlugin<std::unique_ptr<artdaq::MetricPlugin>,
    fhicl::ParameterSet const &>(generator_plugin_spec, ps);
}
