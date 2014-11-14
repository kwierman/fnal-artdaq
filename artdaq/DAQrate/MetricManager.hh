#ifndef artdaq_DAQrate_MetricManager_hh
#define artdaq_DAQrate_MetricManager_hh

// MetricManager class definition file
// Author: Eric Flumerfelt
// Last Modified: 11/14/2014
//
// MetricManager loads a user-specified set of plugins, sends them their configuration,
// and sends them data as it is recieved. It also maintains the state of the plugins
// relative to the application state.

#include "artdaq/Plugins/MetricPlugin.hh"
#include "fhiclcpp/fwd.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


namespace artdaq
{
  class MetricManager;
}

class artdaq::MetricManager
{
public:
  MetricManager();
  MetricManager(MetricManager const&) = delete;
  ~MetricManager();
  MetricManager& operator=(MetricManager const&) = delete;

  void initialize(fhicl::ParameterSet const&);
  void do_start();
  void do_stop();
  void do_pause();
  void do_resume();
  void reinitialize(fhicl::ParameterSet const&);
  void shutdown();

  template<typename T>
  void sendMetric(std::string const& name, T value, std::string const& unit, int level)
  {
    if(initialized_ && running_)
    {
      for(auto & metric : metric_plugins_)
      {
        if(metric->getRunLevel() >= level) {
          try{
            metric->sendMetric(name, value, unit);
          }
          catch (...) {
            mf::LogWarning("MetricManager") << "Error sending value to metric plugin with name "
                                            << metric->getLibName();
          }
        }
      }
    }
    else if(initialized_) {
      mf::LogWarning("MetricManager") << "Attempted to send metric when MetricManager stopped!";
    }
    else {
      mf::LogWarning("MetricManager") << "Attempted to send metric when MetricManager uninitialized!";
    }
  }

private:
  std::vector<std::unique_ptr<artdaq::MetricPlugin>> metric_plugins_;
  bool initialized_;
  bool running_;
};

#endif /* artdaq_DAQrate_MetricManager_hh */
