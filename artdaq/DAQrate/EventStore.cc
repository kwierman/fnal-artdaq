
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

using namespace std;

namespace artdaq {
  const std::string EventStore::EVENT_RATE_STAT_KEY("EventStoreEventRate");
  const std::string EventStore::
    INCOMPLETE_EVENT_STAT_KEY("EventStoreIncompleteEvents");

  EventStore::EventStore(size_t num_fragments_per_event,
                         int store_id,
                         int argc,
                         char * argv[],
                         ART_CMDLINE_FCN * reader,
                         unsigned int seqIDModulus,
                         bool printSummaryStats) :
    id_(store_id),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(1),
    subrun_id_(1),
    events_(),
    queue_(getGlobalQueue()),
    reader_thread_(std::async(std::launch::async, reader, argc, argv)),
    seqIDModulus_(seqIDModulus),
    lastFlushedSeqID_(0),
    highestSeqIDSeen_(0),
    printSummaryStats_(printSummaryStats)
  {
    initStatistics_();
  }

  EventStore::EventStore(size_t num_fragments_per_event,
                         int store_id,
                         const std::string& configString,
                         ART_CFGSTRING_FCN * reader,
                         unsigned int seqIDModulus,
                         bool printSummaryStats) :
    id_(store_id),
    num_fragments_per_event_(num_fragments_per_event),
    run_id_(1),
    subrun_id_(1),
    events_(),
    queue_(getGlobalQueue()),
    reader_thread_(std::async(std::launch::async, reader, configString)),
    seqIDModulus_(seqIDModulus),
    lastFlushedSeqID_(0),
    highestSeqIDSeen_(0),
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

    // The sequenceID is expected to be correct in the incoming fragment.
    // The EventStore will divide it by the seqIDModulus to support the use case
    // of the aggregator which needs to bunch groups of serialized events with
    // continuous sequence IDs together.
    if (pfrag->sequenceID() > highestSeqIDSeen_) {
      highestSeqIDSeen_ = pfrag->sequenceID();
    }
    Fragment::sequence_id_t sequence_id = ((pfrag->sequenceID() - (1 + lastFlushedSeqID_)) / seqIDModulus_) + 1;

    // Find if the right event id is already known to events_ and, if so, where
    // it is.
    EventMap::iterator loc = events_.lower_bound(sequence_id);

    if (loc == events_.end() || events_.key_comp()(sequence_id, loc->first)) {
      // We don't have an event with this id; create one an insert it at loc,
      // and ajust loc to point to the newly inserted event.
      RawEvent_ptr newevent(new RawEvent(run_id_, subrun_id_, pfrag->sequenceID()));
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
      std::cout << "artdaq::EventStore(" << id_ << "): Enqueueing event " << complete_event->sequenceID() << std::endl;
      queue_.enqNowait(complete_event);
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
    std::cout << "EventStore::endOfData(" << id_ << "): Called." << std::endl;
    RawEvent_ptr end_of_data(nullptr);
    queue_.enqNowait(end_of_data);
    return reader_thread_.get();
  }

  void EventStore::flushData()
  {
    std::cout << "EventStore::flushData(" << id_ << "): Called." << std::endl;
    EventMap::iterator loc;
    for (loc = events_.begin(); loc != events_.end(); ++loc) {
      RawEvent_ptr complete_event(loc->second);
      MonitoredQuantityPtr mqPtr = StatisticsCollection::getInstance().
        getMonitoredQuantity(EVENT_RATE_STAT_KEY);
      if (mqPtr.get() != 0) {
        mqPtr->addSample(complete_event->wordCount());
      }
      std::cout << "EventStore::flushData(" << id_ << "): Enqueueing incomplete event" << std::endl;
      queue_.enqNowait(complete_event);
    }
    events_.clear();

    lastFlushedSeqID_ = highestSeqIDSeen_;
  }

  void EventStore::startRun(run_id_t runID) 
  {
    run_id_ = runID;
    subrun_id_ = 1;
    lastFlushedSeqID_ = 0;
    highestSeqIDSeen_ = 0;
  }

  void EventStore::startSubrun() 
  {
    subrun_id_ += 1;
  }

  void EventStore::endRun() 
  {
    RawEvent_ptr endOfRunEvent(new RawEvent(run_id_, subrun_id_, 0));
    std::unique_ptr<artdaq::Fragment> endOfRunFrag(new Fragment(static_cast<size_t>(ceil(sizeof(size_t) /
											 static_cast<double>(sizeof(Fragment::value_type))))));

    endOfRunFrag->setSystemType(Fragment::EndOfRunFragmentType);
    *endOfRunFrag->dataBegin() = id_;
    endOfRunEvent->insertFragment(std::move(endOfRunFrag));

    std::cout << "EventStore(" << id_ << "): Enqueueing an EndRun message." << std::endl;
    queue_.enqNowait(endOfRunEvent);
  }
  void EventStore::endSubrun() 
  {
    RawEvent_ptr endOfSubrunEvent(new RawEvent(run_id_, subrun_id_, 0));
    std::unique_ptr<artdaq::Fragment> endOfSubrunFrag(new Fragment(static_cast<size_t>(ceil(sizeof(size_t) /
											    static_cast<double>(sizeof(Fragment::value_type))))));

    endOfSubrunFrag->setSystemType(Fragment::EndOfSubrunFragmentType);
    *endOfSubrunFrag->dataBegin() = id_;
    endOfSubrunEvent->insertFragment(std::move(endOfSubrunFrag));

    std::cout << "EventStore(" << id_ << "): Enqueueing an EndSubrun message." << std::endl;
    queue_.enqNowait(endOfSubrunEvent);
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
