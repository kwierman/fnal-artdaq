// MetricPlugin.hh: Metric Plugin Interface
// Author: Eric Flumerfelt
// Last Modified: 11/05/2014 (Created)
//
// Defines the interface that any ARTDAQ metric plugin must implement


#ifndef __METRIC_INTERFACE__
#define __METRIC_INTERFACE__

#include <string>
#include <cstdint>
#include "fhiclcpp/ParameterSet.h"

namespace artdaq {
  class MetricPlugin {
 protected:
  int runLevel_;
  fhicl::ParameterSet pset;
 public:
  MetricPlugin(fhicl::ParameterSet const & ps) : pset(ps) {
           runLevel_ = pset.get<int>("level",0);
 }
  virtual ~MetricPlugin() = default;

  virtual void sendMetric(std::string name, std::string value, std::string unit) =0;
  virtual void sendMetric(std::string name, int value, std::string unit) =0;
  virtual void sendMetric(std::string name, double value, std::string unit) =0;
  virtual void sendMetric(std::string name, float value, std::string unit) =0;
  virtual void sendMetric(std::string name, uint32_t value, std::string unit) =0;

  void setRunLevel(int level) { runLevel_ = level; }
  int getRunLevel() { return runLevel_; }

};

} //End namespace artdaq

#endif //End ifndef __METRIC_INTERFACE__
