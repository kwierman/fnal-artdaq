
#include "EventStore.hh"
#include "Perf.hh"
#include <utility>
#include <cstring>
#include <dlfcn.h>
#include "SimpleQueueReader.hh"

using namespace std;

namespace
{
  typedef int (ARTFUL_FCN)(int, char**);

  // Because we can't depend on art, we copied this from hard_cast.h

  inline
  void
  hard_cast(void * src, ARTFUL_FCN* & dest)
  {
    memcpy(&dest, &src, sizeof(ARTFUL_FCN*));
  }

  ARTFUL_FCN* get_artapp()
  {
    // This is hackery, and needs to be made robust.
    void* lptr = dlopen("libartdaq_art", RTLD_LAZY | RTLD_GLOBAL);
    assert(lptr);
    void* fptr = dlsym(lptr, "artmain");
    assert(fptr);
    ARTFUL_FCN* result(0);
    hard_cast(fptr, result);
    return result;
  }

  ARTFUL_FCN* choose_function(Config const& cfg)
  {
    return (cfg.use_artapp_) ? get_artapp() : &artdaq::simpleQueueReaderApp;
  }
}

namespace artdaq
{

  EventStore::EventStore(int src_count, int run, int argc, char* argv[]):
    rank_(0),
    sources_(src_count),
    fragmentIdOffset_(0),
    run_(run),
    firstFragment_(true),
    events_(),
    eventBuildingMonitor_(1, 1),
    queue_(getGlobalQueue()),
    reader_thread_(simpleQueueReaderApp, 0, nullptr)
  { }

  EventStore::EventStore(Config const& conf) :
    rank_(conf.rank_),
    sources_(conf.sources_),
    fragmentIdOffset_(conf.srcStart()),
    run_(conf.run_),
    firstFragment_(true),
    events_(),
    eventBuildingMonitor_(1, 1),
    queue_(getGlobalQueue()),
    reader_thread_(simpleQueueReaderApp, 0, nullptr)
  { }

  EventStore::~EventStore()
  {
    reader_thread_.join();

    eventBuildingMonitor_.calculateStatistics();
    artdaq::MonitoredQuantity::Stats stats;
    eventBuildingMonitor_.getStats(stats);
    std::cout << "EventStore " << rank_
              << ": events processed = "
              << stats.fullSampleCount
              << ", rate = "
              << stats.fullSampleRate
              << " events/sec"
              << ", date rate = "
              << (stats.fullValueRate * sizeof(RawDataType)
                  / 1024.0 / 1024.0)
              << " MB/sec" << std::endl;
  }

  void EventStore::insert(Fragment& ef)
  {
    assert(!ef.empty());
    if (firstFragment_) {
      firstFragment_ = false;
      eventBuildingMonitor_.reset();
    }

    // find the event being built and put the fragment into it,
    // start new event if not already present
    // if the event is complete, delete it and report timing

    RawFragmentHeader* fh = reinterpret_cast<RawFragmentHeader*>(&ef[0]);
    RawDataType event_id = fh->event_id_;

    // update the fragment ID (up to this point, it has been set to the
    // detector rank or the source rank, but now we just want a simple index)
    fh->fragment_id_ -= fragmentIdOffset_;

    RawEvent_ptr rawEventPtr(new RawEvent());
    pair<EventMap::iterator,bool> p =
      events_.insert(EventMap::value_type(event_id, rawEventPtr));

    bool newElementInMap = p.second;
    rawEventPtr = p.first->second;

    if (newElementInMap)
      {
        PerfWriteEvent(EventMeas::START,event_id);

        rawEventPtr->header_.word_count_ = fh->word_count_ +
          (sizeof(RawEventHeader) / sizeof(RawDataType));
        rawEventPtr->header_.run_id_ = run_;
        rawEventPtr->header_.subrun_id_ = 0;
        rawEventPtr->header_.event_id_ = event_id;
      }
    else
      {
        rawEventPtr->header_.word_count_ += fh->word_count_;
      }

    RawEvent::FragmentPtr fp(new Fragment(fh->word_count_));
    memcpy(&(*fp)[0], &ef[0], (fh->word_count_ * sizeof(RawDataType)));
    rawEventPtr->fragments_.push_back(fp);

    if (static_cast<int>(rawEventPtr->fragments_.size()) == sources_)
      {
        PerfWriteEvent(EventMeas::END,event_id);
        events_.erase(p.first);
        queue_.enqNowait( rawEventPtr );

        eventBuildingMonitor_.addSample(rawEventPtr->header_.word_count_);
      }
  }

  void
  EventStore::endOfData()
  {
    RawEvent_ptr end_of_data(0);
    queue_.enqNowait(end_of_data);
  }
}
