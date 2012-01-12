#ifndef eventstore_hhh
#define eventstore_hhh

#include "Config.hh"
#include "FragmentPool.hh"

#include "DAQdata/RawData.hh"
#include "GlobalQueue.hh"
#include "SimpleQueueReader.hh"

#include <map>
#include <memory>
#include <thread>

// bad to get definition for Data from FragmentPool!

namespace artdaq
{
  // Class EventStore represents ...  There must be only one
  // EventStore object in any program, because of the way EventStore
  // interacts with the creation of the shared ConcurrentQueue.

  // EventStore objects are not copyable.

  class EventStore
  {
  public:
    typedef FragmentPool::Data Fragment;
    typedef std::map<RawDataType, RawEvent_ptr> EventMap;

    explicit EventStore(Config const&);
    ~EventStore();

    // The fragment we are given must NOT be empty; it must at least
    // contain the necessary header information.
    void insert(Fragment&);

    // Put the end-of-data marker onto the RawEvent queue.
    void endOfData();

  private:
    EventStore(EventStore const&);            // not implemented
    EventStore& operator=(EventStore const&); // not implemented

    int const      sources_;
    int const      fragmentIdOffset_;
    int const      run_;
    EventMap       events_;
    RawEventQueue& queue_;
    //std::thread    art_thread_;
    std::shared_ptr<SimpleQueueReader> reader_;

  };
}
#endif
