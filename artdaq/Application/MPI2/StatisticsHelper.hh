#ifndef artdaq_Application_MPI2_StatisticsHelper_hh
#define artdaq_Application_MPI2_StatisticsHelper_hh

#include "artdaq/DAQrate/StatisticsCollection.hh"
#include "fhiclcpp/ParameterSet.h"
#include <vector>

namespace artdaq
{
  class StatisticsHelper;
}

class artdaq::StatisticsHelper
{
public:
  StatisticsHelper();
  StatisticsHelper(StatisticsHelper const&) = delete;
  ~StatisticsHelper();
  StatisticsHelper& operator=(StatisticsHelper const&) = delete;

  void addMonitoredQuantityName(std::string const& statKey);
  void addSample(std::string const& statKey, double value);
  void createCollectors(fhicl::ParameterSet const& pset,
                        int defaultReportIntervalFragments,
                        double defaultReportIntervalSeconds,
                        double defaultMonitorWindow);
  void resetStatistics();
  bool readyToReport(std::string const& primaryStatKeyName,
                     size_t currentCount);

private:
  std::vector<std::string> monitored_quantity_name_list_;

  int reporting_interval_fragments_;
  double reporting_interval_seconds_;
  size_t previous_reporting_index_;

};

#endif /* artdaq_Application_MPI2_StatisticsHelper_hh */
