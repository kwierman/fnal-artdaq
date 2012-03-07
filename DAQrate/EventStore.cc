
#include "EventStore.hh"
#include "Perf.hh"
#include <utility>
#include <cstring>
#include <dlfcn.h>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "quiet_mpi.hh"
#include "StatisticsCollection.hh"
#include "SimpleQueueReader.hh"
#include "Utils.hh"

using namespace std;

namespace
{
  typedef int (ARTFUL_FCN)(int, char**);

  // Because we do not want to introduce a compile- or link-time
  // dependence on art, we copied this from hard_cast.h
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

  ARTFUL_FCN* choose_function(bool use_artapp)
  {
    return (use_artapp) ? get_artapp() : &artdaq::simpleQueueReaderApp;
  }
}

namespace artdaq
{
  const std::string EventStore::EVENT_RATE_STAT_KEY("EventStoreEventRate");

  inline int get_mpi_rank()
  {
    int rk(0);
    MPI_Comm_rank(MPI_COMM_WORLD, &rk);
    return rk;
  }

  EventStore::EventStore(size_t num_fragments_per_event,
                         run_id_t run,
                         int argc __attribute__((unused)),
                         char* argv[] __attribute__((unused))):
    id_(0),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(run),
    subrun_id_(0),
    events_(),
    queue_(getGlobalQueue()),
    reader_thread_(simpleQueueReaderApp, 0, nullptr)
  {
    // TODO: Consider doing away with the named local mqPtr, and
    // making use of make_shared<MonitoredQuantity> in the call to
    // addMonitoredQuantity. Maybe modify addMonitoredQuantity to be a
    // variadic template that forwards its function arguments to the
    // constructor of the MonitoredQuantity being made?
    MonitoredQuantityPtr mqPtr(new MonitoredQuantity(1.0, 60.0));
    StatisticsCollection::getInstance().
      addMonitoredQuantity(EVENT_RATE_STAT_KEY, mqPtr);
  }

  EventStore::EventStore(size_t num_fragments_per_event,
                         run_id_t run) :
    id_(get_mpi_rank()),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(run),
    subrun_id_(0),
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
    reportStatistics_();
  }

  void EventStore::insert(FragmentPtr pfrag)
  {
    // We should never get a null pointer, nor should we get a
    // Fragment without a good fragment ID.
    assert(pfrag != nullptr);
    assert(pfrag->fragmentID() != Fragment::InvalidFragmentID);

    // find the event being built and put the fragment into it,
    // start new event if not already present
    // if the event is complete, delete it and report timing

    //RawFragmentHeader* fh = pfrag->fragmentHeader();
    Fragment::event_id_t event_id = pfrag->eventID();

    // The fragmentID is expected to be correct in the incoming
    // fragment; the EventStore has no business changing it in the
    // current design.
    //     // update the fragment ID (up to this point, it has been set to the
    //     // detector rank or the source rank, but now we just want a simple index)
    //     fh->fragment_id_ -= fragmentIdOffset_;

    // Find if the right event id is already known to events_ and, if so, where
    // it is.
    EventMap::iterator loc = events_.lower_bound(event_id);
    if (loc == events_.end() || events_.key_comp()(event_id, loc->first))
      {
        // We don't have an event with this id; create one an insert it at loc,
        // and ajust loc to point to the newly inserted event.
        RawEvent_ptr newevent(new RawEvent(run_id_, subrun_id_, event_id));
        loc = 
          events_.insert(loc, EventMap::value_type(event_id, newevent));
      }

    // Now insert the fragment into the event we have located.
    loc->second->insertFragment(std::move(pfrag));

    if (loc->second->numFragments() == num_fragments_per_event_)
      {
        // This RawEvent is complete; capture it, remove it from the
        // map, report on statistics, and put the shared pointer onto
        // the event queue.
        RawEvent_ptr complete_event(loc->second);
        PerfWriteEvent(EventMeas::END,event_id);
        events_.erase(loc);
        queue_.enqNowait(complete_event);

        MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
          getMonitoredQuantity(EVENT_RATE_STAT_KEY);
        if (mqPtr.get() != 0) {
          mqPtr->addSample(complete_event->wordCount());
        }
      }
  }

  void
  EventStore::endOfData()
  {
    RawEvent_ptr end_of_data(0);
    queue_.enqNowait(end_of_data);
  }

  void
  EventStore::reportStatistics_()
  {
    MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
      getMonitoredQuantity(EVENT_RATE_STAT_KEY);
    if (mqPtr.get() != 0) {
      ostringstream oss;
      oss << EVENT_RATE_STAT_KEY << "_" << setfill('0') << setw(4) << run_id_
          << "_" << setfill('0') << setw(4) << id_ << ".txt";
      std::string filename = oss.str();
      ofstream outStream(filename.c_str());

      mqPtr->waitUntilAccumulatorsHaveBeenFlushed(1.0);
      artdaq::MonitoredQuantity::Stats stats;
      mqPtr->getStats(stats);
      outStream << "EventStore rank " << id_ << ": events processed = "
                << stats.fullSampleCount << " at " << stats.fullSampleRate
                << " events/sec, date rate = "
                << (stats.fullValueRate * sizeof(RawDataType)
                    / 1024.0 / 1024.0) << " MB/sec, duration = "
                << stats.fullDuration << " sec" << std::endl;
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
}
