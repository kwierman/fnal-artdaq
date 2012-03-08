#ifndef artdaq_DAQrate_StatisticsCollection_hh
#define artdaq_DAQrate_StatisticsCollection_hh

#include "artdaq/DAQrate/MonitoredQuantity.hh"
#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace artdaq
{
  typedef std::shared_ptr<MonitoredQuantity> MonitoredQuantityPtr;

  class StatisticsCollection {

  public:

    static StatisticsCollection& getInstance();
    ~StatisticsCollection();

    void addMonitoredQuantity(const std::string& name,
                              MonitoredQuantityPtr mqPtr);
    MonitoredQuantityPtr getMonitoredQuantity(const std::string& name) const;
    void reset();

    void requestStop();
    void run();

  private:

    explicit StatisticsCollection();

    // disallow any copying
    StatisticsCollection(StatisticsCollection const&);  // not implemented
    void operator= (StatisticsCollection const&);  // not implemented

    double calculationInterval_;
    std::map<std::string, MonitoredQuantityPtr> monitoredQuantityMap_;

    bool thread_stop_requested_;
    std::thread* calculation_thread_;
    mutable std::mutex map_mutex_;
  };

}

#endif /* artdaq_DAQrate_StatisticsCollection_hh */
