
#include "artdaq/DAQrate/EventStore.hh"
#include "artdaq/DAQrate/Perf.hh"
#include <utility>
#include <cstring>
#include <dlfcn.h>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "cetlib/exception.h"
#include "artdaq/DAQrate/quiet_mpi.hh"
#include "artdaq/DAQrate/StatisticsCollection.hh"
#include "artdaq/DAQrate/SimpleQueueReader.hh"
#include "artdaq/DAQrate/Utils.hh"

using namespace std;

namespace artdaq
{
  const std::string EventStore::EVENT_RATE_STAT_KEY("EventStoreEventRate");

//   inline int get_mpi_rank()
//   {
//     int rk(0);
//     MPI_Comm_rank(MPI_COMM_WORLD, &rk);
//     return rk;
//   }

  EventStore::EventStore(size_t num_fragments_per_event,
                         run_id_t run,
                         int store_id,
                         int argc,
                         char* argv[],
                         ARTFUL_FCN* reader) :
    id_(store_id),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(run),
    subrun_id_(0),
    events_(),
    queue_(getGlobalQueue()),
    reader_thread_(std::async(std::launch::async, reader, argc, argv))
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

//   EventStore::EventStore(size_t num_fragments_per_event,
//                          run_id_t run,
//                          ARTFUL_FCN* reader) :
//     id_(get_mpi_rank()),
//     num_fragments_per_event_(num_fragments_per_event),
//     run_id_(run),
//     subrun_id_(0),
//     events_(),
//     queue_(getGlobalQueue()),
//     reader_thread_(reader, 0, nullptr)
//   {
//     MonitoredQuantityPtr mqPtr(new MonitoredQuantity(1.0, 60.0));
//     StatisticsCollection::getInstance().
//       addMonitoredQuantity(EVENT_RATE_STAT_KEY, mqPtr);
//   }


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
    Fragment::sequence_id_t sequence_id = pfrag->sequenceID();

    // The fragmentID is expected to be correct in the incoming
    // fragment; the EventStore has no business changing it in the
    // current design.
    //     // update the fragment ID (up to this point, it has been set to the
    //     // detector rank or the source rank, but now we just want a simple index)
    //     fh->fragment_id_ -= fragmentIdOffset_;

    // Find if the right event id is already known to events_ and, if so, where
    // it is.
    EventMap::iterator loc = events_.lower_bound(sequence_id);
    if (loc == events_.end() || events_.key_comp()(sequence_id, loc->first))
      {
        // We don't have an event with this id; create one an insert it at loc,
        // and ajust loc to point to the newly inserted event.
        RawEvent_ptr newevent(new RawEvent(run_id_, subrun_id_, sequence_id));
        loc = 
          events_.insert(loc, EventMap::value_type(sequence_id, newevent));
      }

    // Now insert the fragment into the event we have located.
    loc->second->insertFragment(std::move(pfrag));

    if (loc->second->numFragments() == num_fragments_per_event_)
      {
        // This RawEvent is complete; capture it, remove it from the
        // map, report on statistics, and put the shared pointer onto
        // the event queue.
        RawEvent_ptr complete_event(loc->second);
        PerfWriteEvent(EventMeas::END,sequence_id);
        events_.erase(loc);
        queue_.enqNowait(complete_event);

        MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
          getMonitoredQuantity(EVENT_RATE_STAT_KEY);
        if (mqPtr.get() != 0) {
          mqPtr->addSample(complete_event->wordCount());
        }
      }
  }

  int
  EventStore::endOfData()
  {
    RawEvent_ptr end_of_data(0);
    queue_.enqNowait(end_of_data);
    return reader_thread_.get();
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
