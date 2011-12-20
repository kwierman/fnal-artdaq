#ifndef eventstore_hhh
#define eventstore_hhh

#include "Config.hh"
#include "EventPool.hh"
#include "ConcurrentQueue.hh"
#include "DAQdata/RawData.hh"
#include "DAQrate/GlobalQueue.hh"

#include <map>
#include <memory>
#include <thread>

// bad to get definition for Data from EventPool!

namespace artdaq
{

  class EventStore
  {
  public:
    typedef FragmentPool::Data Data;
    typedef std::map<ElementType,int> EventMap;

    EventStore(Config const&);
    ~EventStore();

    void operator()(Data const&);
    void run();

  private:
    int sources_;
    EventMap events_;
    RawEventQueue  queue_;

    bool thread_stop_requested_;
    std::thread* reader_thread_;
  };
}
#endif
