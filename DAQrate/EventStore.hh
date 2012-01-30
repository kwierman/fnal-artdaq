#ifndef eventstore_hhh
#define eventstore_hhh

#include "Config.hh"
#include "Fragment.hh"

#include "DAQdata/RawData.hh"
#include "GlobalQueue.hh"

#include <map>
#include <memory>
#include <thread>

namespace artdaq
{
  // Class EventStore represents ...  There must be only one
  // EventStore object in any program, because of the way EventStore
  // interacts with the creation of the shared ConcurrentQueue.

  // EventStore objects are not copyable.

  class EventStore
  {
  public:
    typedef std::map<RawDataType, RawEvent_ptr> EventMap;

    static const std::string EVENT_RATE_STAT_KEY;

    explicit EventStore(Config const&);
    EventStore(int src_count, int run, int argc, char* argv[]);

    ~EventStore();

    // The pointer we are given must NOT be null, and the Fragment to
    // which it points must NOT be empty; the Fragment must at least
    // contain the necessary header information.
    void insert(FragmentPtr &&);

    // Put the end-of-data marker onto the RawEvent queue.
    void endOfData();

  private:
    EventStore(EventStore const&);            // not implemented
    EventStore& operator=(EventStore const&); // not implemented

    int const      rank_;
    int const      sources_;
    int const      fragmentIdOffset_;
    int const      run_;
    EventMap       events_;
    RawEventQueue& queue_;
    std::thread    reader_thread_;
  };
}
#endif
