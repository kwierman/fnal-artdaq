#ifndef DAQ_GLOBAL_QUEUE_HH
#define DAQ_GLOBAL_QUEUE_HH

#include "DAQrate/ConcurrentQueue.hh"
#include "DAQdata/RawData.hh"
#include <memory>

namespace artdaq
{
  typedef std::shared_ptr<RawEvent> RawEvent_ptr;
  typedef daqrate::ConcurrentQueue<RawEvent_ptr> RawEventQueue;

  void setQueue(RawEventQueue&);
  RawEventQueue& getQueue();
}

#endif
