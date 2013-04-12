
#include "artdaq/DAQrate/EventStore.hh"
#ifndef ARTDAQ_NO_PERF
#include "artdaq/DAQrate/Perf.hh"
#endif
#include <utility>
#include <cstring>
#include <dlfcn.h>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "cetlib/exception.h"
#include "artdaq/DAQrate/StatisticsCollection.hh"
#include "artdaq/DAQrate/SimpleQueueReader.hh"
#include "artdaq/DAQrate/Utils.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace std;

namespace artdaq {
  const std::string EventStore::EVENT_RATE_STAT_KEY("EventStoreEventRate");
  const std::string EventStore::
    INCOMPLETE_EVENT_STAT_KEY("EventStoreIncompleteEvents");

  EventStore::EventStore(size_t num_fragments_per_event,
                         run_id_t run,
                         int store_id,
                         int argc,
                         char * argv[],
                         ART_CMDLINE_FCN * reader,
                         bool printSummaryStats) :
    id_(store_id),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(run),
    subrun_id_(0),
    events_(),
    queue_(getGlobalQueue(50)),
    reader_thread_(std::async(std::launch::async, reader, argc, argv)),
    enq_timeout_(5.0),
    printSummaryStats_(printSummaryStats)
  {
    initStatistics_();
  }

  EventStore::EventStore(size_t num_fragments_per_event,
                         run_id_t run,
                         int store_id,
                         const std::string& configString,
                         ART_CFGSTRING_FCN * reader,
                         bool printSummaryStats) :
    id_(store_id),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(run),
    subrun_id_(0),
    events_(),
    queue_(getGlobalQueue(50)),
    reader_thread_(std::async(std::launch::async, reader, configString)),
    enq_timeout_(5.0),
    printSummaryStats_(printSummaryStats)
  {
    initStatistics_();
  }

  EventStore::EventStore(size_t num_fragments_per_event,
                         run_id_t run,
                         int store_id,
                         int argc,
                         char * argv[],
                         ART_CMDLINE_FCN * reader,
                         int max_art_queue_size,
                         double enq_timeout_sec,
                         bool printSummaryStats) :
    id_(store_id),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(run),
    subrun_id_(0),
    events_(),
    queue_(getGlobalQueue(max_art_queue_size)),
    reader_thread_(std::async(std::launch::async, reader, argc, argv)),
    enq_timeout_(enq_timeout_sec),
    printSummaryStats_(printSummaryStats)
  {
    initStatistics_();
  }

  EventStore::EventStore(size_t num_fragments_per_event,
                         run_id_t run,
                         int store_id,
                         const std::string& configString,
                         ART_CFGSTRING_FCN * reader,
                         int max_art_queue_size,
                         double enq_timeout_sec,
                         bool printSummaryStats) :
    id_(store_id),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(run),
    subrun_id_(0),
    events_(),
    queue_(getGlobalQueue(max_art_queue_size)),
    reader_thread_(std::async(std::launch::async, reader, configString)),
    enq_timeout_(enq_timeout_sec),
    printSummaryStats_(printSummaryStats)
  {
    initStatistics_();
  }

  EventStore::~EventStore()
  {
    if (printSummaryStats_) {
      reportStatistics_();
    }
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
    if (loc == events_.end() || events_.key_comp()(sequence_id, loc->first)) {
      // We don't have an event with this id; create one an insert it at loc,
      // and ajust loc to point to the newly inserted event.
      RawEvent_ptr newevent(new RawEvent(run_id_, subrun_id_, sequence_id));
      loc =
        events_.insert(loc, EventMap::value_type(sequence_id, newevent));
    }
    // Now insert the fragment into the event we have located.
    loc->second->insertFragment(std::move(pfrag));
    if (loc->second->numFragments() == num_fragments_per_event_) {
      // This RawEvent is complete; capture it, remove it from the
      // map, report on statistics, and put the shared pointer onto
      // the event queue.
      RawEvent_ptr complete_event(loc->second);
      complete_event->markComplete();
#ifndef ARTDAQ_NO_PERF
      PerfWriteEvent(EventMeas::END, sequence_id);
#endif
      events_.erase(loc);
      // 13-Dec-2012, KAB - this monitoring needs to come before
      // the enqueueing of the event lest it be empty by the 
      // time that we ask for the word count.
      MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
        getMonitoredQuantity(EVENT_RATE_STAT_KEY);
      if (mqPtr.get() != 0) {
        mqPtr->addSample(complete_event->wordCount());
      }
      bool enqSuccess = queue_.enqTimedWait(complete_event, enq_timeout_);
      if (! enqSuccess) {
        mf::LogError("EventStore") << "Enqueueing event " << sequence_id
                                   << " FAILED , queue size = " << queue_.size();
      }
    }
    MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
      getMonitoredQuantity(INCOMPLETE_EVENT_STAT_KEY);
    if (mqPtr.get() != 0) {
      mqPtr->addSample(events_.size());
    }
  }

  int
  EventStore::endOfData()
  {
    bool enqSuccess;
    // 26-Mar-2013, KAB: This will need to change once we get better
    // end run handling, but for now let's at least drain the event
    // store when we are shutting down (ending the run).
    mf::LogDebug("EventStore")
      << "Flushing " << events_.size() << " stale events from the EventStore.";
    EventMap::iterator loc;
    for (loc = events_.begin(); loc != events_.end(); ++loc) {
      RawEvent_ptr complete_event(loc->second);
      MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
        getMonitoredQuantity(EVENT_RATE_STAT_KEY);
      if (mqPtr.get() != 0) {
        mqPtr->addSample(complete_event->wordCount());
      }
      enqSuccess = queue_.enqTimedWait(complete_event, enq_timeout_);
      if (! enqSuccess) {
        mf::LogError("EventStore") << "Enqueueing event "
                                   << complete_event->sequenceID()
                                   << " FAILED , queue size = "
                                   << queue_.size();
      }
    }
    events_.clear();
    mf::LogDebug("EventStore")
      << "Done flushing stale events from the EventStore.";

    RawEvent_ptr end_of_data(0);
    enqSuccess = queue_.enqTimedWait(end_of_data, enq_timeout_);
    if (! enqSuccess) {
      mf::LogError("EventStore") << "Enqueueing END_OF_DATA event "
                                 << end_of_data->sequenceID()
                                 << " FAILED , queue size = "
                                 << queue_.size();
    }
    return reader_thread_.get();
  }

  void
  EventStore::initStatistics_()
  {
    MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
      getMonitoredQuantity(EVENT_RATE_STAT_KEY);
    if (mqPtr.get() == 0) {
      mqPtr.reset(new MonitoredQuantity(3.0, 300.0));
      StatisticsCollection::getInstance().
        addMonitoredQuantity(EVENT_RATE_STAT_KEY, mqPtr);
    }
    mqPtr->reset();

    mqPtr = StatisticsCollection::getInstance().
      getMonitoredQuantity(INCOMPLETE_EVENT_STAT_KEY);
    if (mqPtr.get() == 0) {
      mqPtr.reset(new MonitoredQuantity(3.0, 300.0));
      StatisticsCollection::getInstance().
        addMonitoredQuantity(INCOMPLETE_EVENT_STAT_KEY, mqPtr);
    }
    mqPtr->reset();
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
      mqPtr->waitUntilAccumulatorsHaveBeenFlushed(3.0);
      artdaq::MonitoredQuantity::Stats stats;
      mqPtr->getStats(stats);
      outStream << "EventStore rank " << id_ << ": events processed = "
                << stats.fullSampleCount << " at " << stats.fullSampleRate
                << " events/sec, date rate = "
                << (stats.fullValueRate * sizeof(RawDataType)
                    / 1024.0 / 1024.0) << " MB/sec, duration = "
                << stats.fullDuration << " sec" << std::endl
                << "    minimum event size = "
                << (stats.fullValueMin * sizeof(RawDataType)
                    / 1024.0 / 1024.0)
                << " MB, maximum event size = "
                << (stats.fullValueMax * sizeof(RawDataType)
                    / 1024.0 / 1024.0)
                << " MB" << std::endl;
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

    mqPtr = StatisticsCollection::getInstance().
      getMonitoredQuantity(INCOMPLETE_EVENT_STAT_KEY);
    if (mqPtr.get() != 0) {
      ostringstream oss;
      oss << INCOMPLETE_EVENT_STAT_KEY << "_" << setfill('0')
          << setw(4) << run_id_
          << "_" << setfill('0') << setw(4) << id_ << ".txt";
      std::string filename = oss.str();
      ofstream outStream(filename.c_str());
      mqPtr->waitUntilAccumulatorsHaveBeenFlushed(3.0);
      artdaq::MonitoredQuantity::Stats stats;
      mqPtr->getStats(stats);
      outStream << "EventStore rank " << id_ << ": fragments processed = "
                << stats.fullSampleCount << " at " << stats.fullSampleRate
                << " fragments/sec, average incomplete event count = "
                << stats.fullValueAverage << " duration = "
                << stats.fullDuration << " sec" << std::endl
                << "    minimum incomplete event count = "
                << stats.fullValueMin << ", maximum incomplete event count = "
                << stats.fullValueMax << std::endl;
      bool foundTheStart = false;
      for (int idx = 0; idx < (int) stats.recentBinnedDurations.size(); ++idx) {
        if (stats.recentBinnedDurations[idx] > 0.0) {
          foundTheStart = true;
        }
        if (foundTheStart && stats.recentBinnedSampleCounts[idx] > 0.0) {
          outStream << "  " << std::fixed << std::setprecision(3)
                    << stats.recentBinnedEndTimes[idx]
                    << ": " << stats.recentBinnedSampleCounts[idx]
                    << " fragments at "
                    << (stats.recentBinnedSampleCounts[idx] /
                        stats.recentBinnedDurations[idx])
                    << " fragments/sec, average incomplete event count = "
                    << (stats.recentBinnedValueSums[idx] /
                        stats.recentBinnedSampleCounts[idx])
                    << ", bin size = "
                    << stats.recentBinnedDurations[idx]
                    << " sec" << std::endl;
        }
      }
      outStream << "Incomplete count now = " << events_.size() << std::endl;
      outStream.close();
    }
  }
}
