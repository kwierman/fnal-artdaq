#ifndef artdaq_DAQrate_EventStore_hh
#define artdaq_DAQrate_EventStore_hh

#include "artdaq/DAQdata/RawEvent.hh"
#include "artdaq/DAQrate/GlobalQueue.hh"

#include <map>
#include <memory>
//#include <thread>
#include <future>
#include <stdint.h>

namespace artdaq
{

  // An EventStore is given Fragments, which it collects until it
  // finds it has a complete RawEvent. When a complete RawEvent is
  // assembled, the EventStore puts it onto the global RawEvent queue.
  // There should be only one EventStore per process; an MPI program
  // can thus have multiple EventStores. By construction, each
  // EventStore will only deal with events (and fragments) from a
  // single run.
  //
  // The EventStore is also responsible for starting the thread that
  // will be popping events off the global queue. This is so that the
  // EventStore is guaranteed to live long enough to allow the global
  // queue to be drained. The current implementation uses only a free
  // function as the 'thread function' for this thread.
  //
  // A future enhancement of EventStore may make it be able to move
  // from handling run X to handling run Y; such an enhancement will
  // have to include how to deal with any incomplete events in storage
  // at the time of the introduction of the new run.

  class EventStore
  {
  public:
    typedef int (ARTFUL_FCN)(int, char**);
    typedef RawEvent::run_id_t      run_id_t;
    typedef RawEvent::subrun_id_t   subrun_id_t;
    typedef Fragment::event_id_t    event_id_t;
    typedef std::map<event_id_t, RawEvent_ptr> EventMap;

    static const std::string EVENT_RATE_STAT_KEY;

    EventStore() = delete;
    EventStore(EventStore const&) = delete;
    EventStore& operator=(EventStore const&) = delete;

    // This constructor is obsolete; please modify your code to use
    // the c'tor below it...
    EventStore(int, int, int, char**) :
      id_(),
      num_fragments_per_event_(),
      run_id_(),
      subrun_id_(),
      events_(),
      queue_(getGlobalQueue()),
      reader_thread_()
    {
      throw "This constructor is obsolete\n";
    }

    // Create an EventStore that uses 'reader' as the function to be
    // executed by the thread this EventStore will spawn.
    EventStore(size_t num_fragments_per_event, run_id_t run,
               int store_id, int argc, char* argv[],
               ARTFUL_FCN* reader);

    // Give ownership of the Fragment to the EventStore. The pointer
    // we are given must NOT be null, and the Fragment to which it
    // points must NOT be empty; the Fragment must at least contain
    // the necessary header information.
    void insert(FragmentPtr pfrag);

    // Put the end-of-data marker onto the RawEvent queue, and wait
    // for the reader function to exit, returning the value the reader
    // returned.
    int endOfData();

  private:
    // id_ is the unique identifier of this object; MPI programs will
    // use the MPI rank to fill in this value.
    int const      id_;
    size_t  const  num_fragments_per_event_;
    run_id_t const run_id_;
    subrun_id_t const subrun_id_;
    EventMap       events_;
    RawEventQueue& queue_;
    std::future<int> reader_thread_;

    void reportStatistics_();
  };
}
#endif /* artdaq_DAQrate_EventStore_hh */
