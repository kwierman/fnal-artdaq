#ifndef artdaq_DAQrate_GlobalQueue_hh
#define artdaq_DAQrate_GlobalQueue_hh

#include "artdaq/DAQrate/ConcurrentQueue.hh"
#include "artdaq/DAQdata/RawEvent.hh"
#include <memory>

namespace artdaq {
  typedef std::shared_ptr<RawEvent> RawEvent_ptr;
  typedef daqrate::ConcurrentQueue<RawEvent_ptr> RawEventQueue;

  // The first thread to call getGlobalQueue() causes the creation of
  // the queue. The queue will be destroyed at static destruction
  // time.
  RawEventQueue & getGlobalQueue();
}

#endif /* artdaq_DAQrate_GlobalQueue_hh */
