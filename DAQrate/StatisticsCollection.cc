#include "StatisticsCollection.hh"

namespace artdaq {

  /**
   * Returns the single instance of the StatisticsCollection.
   *
   * @returns StatisticsCollection instance.
   */
  StatisticsCollection& StatisticsCollection::getInstance()
  {
    static StatisticsCollection singletonInstance;
    return singletonInstance;
  }

  /**
   * Constructs an instance of StatisticsCollection.
   */
  StatisticsCollection::StatisticsCollection() : calculationInterval_(1.0)
  {
    thread_stop_requested_ = false;
    calculation_thread_ = new std::thread(std::bind(&StatisticsCollection::run, this));
  }

  StatisticsCollection::~StatisticsCollection()
  {
    // stop and clean up the thread
    requestStop();
    calculation_thread_->join();
    delete calculation_thread_;
  }

  void StatisticsCollection::
  addMonitoredQuantity(const std::string& name,
                       MonitoredQuantityPtr mqPtr)
  {
    std::lock_guard<std::mutex> scopedLock(map_mutex_);
    monitoredQuantityMap_[name] = mqPtr;
  }

  MonitoredQuantityPtr
  StatisticsCollection::getMonitoredQuantity(const std::string& name) const
  {
    std::lock_guard<std::mutex> scopedLock(map_mutex_);
    MonitoredQuantityPtr emptyResult;

    std::map<std::string, MonitoredQuantityPtr>::const_iterator iter;
    iter = monitoredQuantityMap_.find(name);
    if (iter == monitoredQuantityMap_.end()) {return emptyResult;}
    return iter->second;
  }

  void StatisticsCollection::reset()
  {
    std::lock_guard<std::mutex> scopedLock(map_mutex_);

    std::map<std::string, MonitoredQuantityPtr>::const_iterator iter;
    std::map<std::string, MonitoredQuantityPtr>::const_iterator iterEnd;
    iterEnd = monitoredQuantityMap_.end();
    for (iter = monitoredQuantityMap_.begin(); iter != iterEnd; ++iter) {
      iter->second->reset();
    }
  }

  void StatisticsCollection::requestStop()
  {
    thread_stop_requested_ = true;
  }

  void StatisticsCollection::run()
  {
    while (! thread_stop_requested_) {
      long useconds = static_cast<long>(calculationInterval_ * 1000000);
      usleep(useconds);

      {
        std::lock_guard<std::mutex> scopedLock(map_mutex_);

        std::map<std::string, MonitoredQuantityPtr>::const_iterator iter;
        std::map<std::string, MonitoredQuantityPtr>::const_iterator iterEnd;
        iterEnd = monitoredQuantityMap_.end();
        for (iter = monitoredQuantityMap_.begin(); iter != iterEnd; ++iter) {
          iter->second->calculateStatistics();
        }
      }
    }
  }

}
