// MetricManager.cc: MetricManager class implementation file
// Author: Eric Flumerfelt
// Last Modified: 11/14/2014
//
// MetricManager loads a user-specified set of plugins, sends them their configuration,
// and sends them data as it is recieved. It also maintains the state of the plugins
// relative to the application state.

#include "artdaq/DAQrate/MetricManager.hh"
#include "artdaq/Plugins/makeMetricPlugin.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"

artdaq::MetricManager::
MetricManager() : metric_plugins_(0), initialized_(false), running_(false) { }

artdaq::MetricManager::~MetricManager()
{
  shutdown();
}

void artdaq::MetricManager::initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("MetricManager") << "Confiugring metrics with parameter set:\n" << pset.to_string();
  std::vector<std::string> names = pset.get_pset_keys();
  for(auto name : names)
    {
      try {
      mf::LogDebug("MetricManager") << "Constructing metric plugin with name " << name;
      fhicl::ParameterSet plugin_pset = pset.get<fhicl::ParameterSet>(name);
      metric_plugins_.push_back(makeMetricPlugin(
          plugin_pset.get<std::string>("metricPluginType",""), plugin_pset));
      }
      catch(...) {
        mf::LogWarning("StatisticsHelper") << "Error loading plugin with name " << name;
      }
    }
  initialized_ = true;
}

void artdaq::MetricManager::do_start()
{
  for(auto & metric : metric_plugins_)
    {
      metric->startMetrics();
    }
  running_ = true;
}

void artdaq::MetricManager::do_stop()
{
  for(auto & metric : metric_plugins_)
    {
      metric->stopMetrics();
    }
  running_ = false;
}

void artdaq::MetricManager::do_pause() { do_stop(); }
void artdaq::MetricManager::do_resume() { do_start(); }

void artdaq::MetricManager::reinitialize(fhicl::ParameterSet const& pset)
{
  shutdown();
  initialize(pset);
}

void artdaq::MetricManager::shutdown()
{
  do_stop();
  for(auto & i : metric_plugins_)
    {
      i.reset(nullptr);
    }
  initialized_ = false;
}
