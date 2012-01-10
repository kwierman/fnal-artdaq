#ifndef eventstore_hhh
#define eventstore_hhh

#include "Config.hh"
#include "FragmentPool.hh"

#include "DAQdata/RawData.hh"
#include "GlobalQueue.hh"
#include "SimpleQueueReader.hh"

#include <map>
#include <memory>

// bad to get definition for Data from FragmentPool!

namespace artdaq
{
  // Class EventStore represents ...  There must be only one
  // EventStore object in any program, because of the way EventStore
  // interacts with the creation of the shared ConcurrentQueue.

  class EventStore
  {
  public:
    typedef FragmentPool::Data Fragment;
    typedef std::map<RawDataType, RawEvent_ptr> EventMap;

    explicit EventStore(Config const&);


    void operator()(Fragment&);

  private:
    int const      sources_;
    int const      fragmentIdOffset_;
    int const      run_;
    EventMap       events_;
    RawEventQueue& queue_;
    std::shared_ptr<SimpleQueueReader> reader_;
  };
}
#endif
