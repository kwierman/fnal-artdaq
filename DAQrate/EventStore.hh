#ifndef eventstore_hhh
#define eventstore_hhh

#include "Config.hh"
#include "EventPool.hh"
#include "ConcurrentQueue.hh"
#include "DAQdata/RawData.hh"
#include "DAQrate/GlobalQueue.hh"
#include "SimpleQueueReader.hh"

#include <map>
#include <memory>

// bad to get definition for Data from EventPool!

namespace artdaq
{
  class EventStore
  {
  public:
    typedef FragmentPool::Data Data;
    typedef std::map<RawDataType, std::shared_ptr<RawEvent> > EventMap;

    explicit EventStore(Config const&);

    void operator()(Data const&);

  private:
    int sources_;
    int run_;
    EventMap events_;
    RawEventQueue  queue_;
    std::shared_ptr<SimpleQueueReader> reader_;
  };
}
#endif
