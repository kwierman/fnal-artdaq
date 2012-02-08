#ifndef DAQ_GLOBAL_QUEUE_HH
#define DAQ_GLOBAL_QUEUE_HH

#include "DAQrate/ConcurrentQueue.hh"
#include "DAQdata/RawData.hh"
#include "DAQdata/RawEvent.hh"
#include <memory>

namespace artdaq
{
  typedef std::shared_ptr<RawEvent> RawEvent_ptr;
  typedef daqrate::ConcurrentQueue<RawEvent_ptr> RawEventQueue;

  // The first thread to call getGlobalQueue() causes the creation of
  // the queue. The queue will be destroyed at static destruction
  // time.
  RawEventQueue& getGlobalQueue();
}

#endif
