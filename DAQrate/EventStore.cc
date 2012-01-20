
#include "EventStore.hh"
#include "Perf.hh"
#include <utility>
#include <cstring>
#include <dlfcn.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "SimpleQueueReader.hh"
#include "StatisticsCollection.hh"

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
  const std::string EventStore::EVENT_RATE_STAT_KEY("EventStoreEventRate");

   EventStore::EventStore(int src_count,
                          int run,
                          int argc __attribute__((unused)),
                          char* argv[] __attribute__((unused))):
    rank_(0),
    sources_(src_count),
    fragmentIdOffset_(0),
    run_(run),
    events_(),
    queue_(getGlobalQueue()),
    reader_thread_(simpleQueueReaderApp, 0, nullptr)
  {
    MonitoredQuantityPtr mqPtr(new MonitoredQuantity(1.0, 60.0));
    StatisticsCollection::getInstance().
      addMonitoredQuantity(EVENT_RATE_STAT_KEY, mqPtr);
  }

  EventStore::EventStore(Config const& conf) :
    rank_(conf.rank_),
    sources_(conf.sources_),
    fragmentIdOffset_(conf.srcStart()),
    run_(conf.run_),
    events_(),
    queue_(getGlobalQueue()),
    reader_thread_(simpleQueueReaderApp, 0, nullptr)
  {
    MonitoredQuantityPtr mqPtr(new MonitoredQuantity(1.0, 60.0));
    StatisticsCollection::getInstance().
      addMonitoredQuantity(EVENT_RATE_STAT_KEY, mqPtr);
  }

  EventStore::~EventStore()
  {
    reader_thread_.join();

    MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
      getMonitoredQuantity(EVENT_RATE_STAT_KEY);
    if (mqPtr.get() != 0) {
      ostringstream oss;
      oss << EVENT_RATE_STAT_KEY << "_" << setfill('0') << setw(4) << run_
          << "_" << setfill('0') << setw(4) << rank_ << ".txt";
      std::string filename = oss.str();
      ofstream outStream(filename.c_str());

      mqPtr->waitUntilAccumulatorsHaveBeenFlushed(1.0);
      artdaq::MonitoredQuantity::Stats stats;
      mqPtr->getStats(stats);
      outStream << "EventStore rank " << rank_ << ": events processed = "
                << stats.fullSampleCount << " at " << stats.fullSampleRate
                << " events/sec, date rate = "
                << (stats.fullValueRate * sizeof(RawDataType)
                    / 1024.0 / 1024.0) << " MB/sec" << std::endl;
      bool foundTheStart = false;
      for (int idx = 0; idx < (int) stats.recentBinnedDurations.size(); ++idx) {
        if (stats.recentBinnedDurations[idx] > 0.0) {
          foundTheStart = true;
        }
        if (foundTheStart) {
          outStream << "  " << std::fixed << std::setprecision(3)
                    << stats.recentBinnedEndTimes[idx]
                    << ": " << stats.recentBinnedSampleCounts[idx]
                    << " events at "
                    << (stats.recentBinnedSampleCounts[idx] /
                        stats.recentBinnedDurations[idx])
                    << " events/sec, data rate = "
                    << (stats.recentBinnedValueSums[idx] *
                        sizeof(RawDataType) / 1024.0 / 1024.0 /
                        stats.recentBinnedDurations[idx])
                    << " MB/sec, bin size = "
                    << stats.recentBinnedDurations[idx]
                    << " sec" << std::endl;
        }
      }
      outStream.close();
    }
  }

  void EventStore::insert(Fragment& ef)
  {
    assert(!ef.empty());

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

        MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
          getMonitoredQuantity(EVENT_RATE_STAT_KEY);
        if (mqPtr.get() != 0) {
          mqPtr->addSample(rawEventPtr->header_.word_count_);
        }
      }
  }

  void
  EventStore::endOfData()
  {
    RawEvent_ptr end_of_data(0);
    queue_.enqNowait(end_of_data);
  }
}
