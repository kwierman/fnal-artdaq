#include "artdaq/Application/MPI2/EventBuilderCore.hh"
#include "art/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq/DAQrate/EventStore.hh"
#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQrate/SimpleQueueReader.hh"
#include "artdaq/Utilities/SimpleLookupPolicy.h"
#include "artdaq/DAQdata/NetMonHeader.hh"

const std::string artdaq::EventBuilderCore::INPUT_FRAGMENTS_STAT_KEY("EventBuilderCoreInputFragments");
const std::string artdaq::EventBuilderCore::INPUT_WAIT_STAT_KEY("EventBuilderCoreInputWaitTime");
const std::string artdaq::EventBuilderCore::STORE_EVENT_WAIT_STAT_KEY("EventBuilderCoreStoreEventWaitTime");

/**
 * Constructor.
 */
artdaq::EventBuilderCore::EventBuilderCore(int mpi_rank, MPI_Comm local_group_comm) :
  mpi_rank_(mpi_rank), local_group_comm_(local_group_comm),
  data_sender_count_(0), art_initialized_(false),
  stop_requested_(false), pause_requested_(false), run_is_paused_(false)
{
  mf::LogDebug("EventBuilderCore") << "Constructor";
  statsHelper_.addMonitoredQuantityName(INPUT_FRAGMENTS_STAT_KEY);
  statsHelper_.addMonitoredQuantityName(INPUT_WAIT_STAT_KEY);
  statsHelper_.addMonitoredQuantityName(STORE_EVENT_WAIT_STAT_KEY);
}

/**
 * Destructor.
 */
artdaq::EventBuilderCore::~EventBuilderCore()
{
  mf::LogDebug("EventBuilderCore") << "Destructor";
}

void artdaq::EventBuilderCore::initializeEventStore()
{
  if (use_art_) {
    artdaq::EventStore::ART_CFGSTRING_FCN * reader = &artapp_string_config;
    event_store_ptr_.reset(new artdaq::EventStore(expected_fragments_per_event_, 1,
						  mpi_rank_, init_string_,
						  reader, 20, 5.0, print_event_store_stats_));
    art_initialized_ = true;
  }
  else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
    char * dummyArgs[1] { "SimpleQueueReader" };
#pragma GCC diagnostic pop
    artdaq::EventStore::ART_CMDLINE_FCN * reader = &artdaq::simpleQueueReaderApp;
    event_store_ptr_.reset(new artdaq::EventStore(expected_fragments_per_event_, 1,
						  mpi_rank_, 1, dummyArgs,
						  reader, 20, 5.0, print_event_store_stats_));
  }
}

/**
 * Processes the initialize request.
 */
bool artdaq::EventBuilderCore::initialize(fhicl::ParameterSet const& pset)
{
  init_string_ = pset.to_string();
  mf::LogDebug("EventBuilderCore") << "initialize method called with DAQ "
                               << "ParameterSet = \"" << init_string_ << "\".";

  // pull out the relevant parts of the ParameterSet
  fhicl::ParameterSet daq_pset;
  try {
    daq_pset = pset.get<fhicl::ParameterSet>("daq");
  }
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "Unable to find the DAQ parameters in the initialization "
      << "ParameterSet: \"" + pset.to_string() + "\".";
    return false;
  }
  fhicl::ParameterSet evb_pset;
  try {
    evb_pset = daq_pset.get<fhicl::ParameterSet>("event_builder");
  }
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "Unable to find the event_builder parameters in the DAQ "
      << "initialization ParameterSet: \"" + daq_pset.to_string() + "\".";
    return false;
  }

  // determine the data receiver parameters
  try {
    max_fragment_size_words_ = daq_pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \""
      << daq_pset.to_string() << "\".";
    return false;
  }
  try {mpi_buffer_count_ = evb_pset.get<size_t>("mpi_buffer_count");}
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "The  mpi_buffer_count parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  try {
    first_data_sender_rank_ = evb_pset.get<size_t>("first_fragment_receiver_rank");
  }
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "The first_fragment_receiver_rank parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  try {data_sender_count_ = evb_pset.get<size_t>("fragment_receiver_count");}
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "The fragment_receiver_count parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  try {
    expected_fragments_per_event_ =
      evb_pset.get<size_t>("expected_fragments_per_event");}
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "The expected_fragments_per_event parameter was not specified "
      << "in the event_builder initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }

  // other parameters
  try {use_art_ = evb_pset.get<bool>("use_art");}
  catch (...) {
    mf::LogError("EventBuilderCore")
      << "The use_art parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  print_event_store_stats_ = evb_pset.get<bool>("print_event_store_stats", false);
  inRunRecvTimeoutUSec_=evb_pset.get<size_t>("inrun_recv_timeout_usec",    100000);
  endRunRecvTimeoutUSec_=evb_pset.get<size_t>("endrun_recv_timeout_usec",20000000);
  pauseRunRecvTimeoutUSec_=evb_pset.get<size_t>("endrun_recv_timeout_usec",3000000);

  // fetch the monitoring parameters and create the MonitoredQuantity instances
  statsHelper_.createCollectors(evb_pset, 100, 20.0, 60.0);

  /* Once art has been initialized we can't tear it down or change it's
     configuration.  We'll keep track of when we have initialized it.  Once it
     has been initialized we need to verify that the configuration is the same
     every subsequent time we transition through the init state.  If the
     config changes we have to throw up our hands and bail out.
  */
  if (art_initialized_ == false) {
    this->initializeEventStore();
    fhicl::ParameterSet tmp = pset;
    tmp.erase("daq");
    previous_pset_ = tmp;
  } else {
    fhicl::ParameterSet tmp = pset;
    tmp.erase("daq");
    if (tmp != previous_pset_) {
      mf::LogError("EventBuilderCore")
	<< "The art configuration can not be altered after art "
	<< "has been configured.";
      return false;
    }
  }

  return true;
}

bool artdaq::EventBuilderCore::start(art::RunID id)
{
  stop_requested_.store(false);
  pause_requested_.store(false);
  run_is_paused_.store(false);
  run_id_ = id;
  eod_fragments_received_ = 0;
  fragment_count_in_run_ = 0;
  statsHelper_.resetStatistics();
  flush_mutex_.lock();
  event_store_ptr_->startRun(id.run());

  mf::LogDebug("EventBuilderCore") << "Started run " << run_id_.run();
  return true;
}

bool artdaq::EventBuilderCore::stop()
{
  mf::LogDebug("EventBuilderCore") << "Stopping run " << run_id_.run();
  bool endSucceeded;
  int attemptsToEnd;

  // 21-Jun-2013, KAB - the stop_requested_ variable must be set
  // before the flush lock so that the processFragments loop will
  // exit (after the timeout), the lock will be released (in the
  // processFragments method), and this method can continue.
  stop_requested_.store(true);

  flush_mutex_.lock();
  if (! run_is_paused_.load()) {
    endSucceeded = false;
    attemptsToEnd = 1;
    endSucceeded = event_store_ptr_->endSubrun();
    while (! endSucceeded && attemptsToEnd < 3) {
      ++attemptsToEnd;
      mf::LogDebug("EventBuilderCore") << "Retrying EventStore::endSubrun()";
      endSucceeded = event_store_ptr_->endSubrun();
    }
    if (! endSucceeded) {
      mf::LogError("EventBuilderCore")
        << "EventStore::endSubrun in stop method failed after three tries.";
    }
  }

  endSucceeded = false;
  attemptsToEnd = 1;
  endSucceeded = event_store_ptr_->endRun();
  while (! endSucceeded && attemptsToEnd < 3) {
    ++attemptsToEnd;
    mf::LogDebug("EventBuilderCore") << "Retrying EventStore::endRun()";
    endSucceeded = event_store_ptr_->endRun();
  }
  if (! endSucceeded) {
    mf::LogError("EventBuilderCore")
      << "EventStore::endRun in stop method failed after three tries.";
  }

  flush_mutex_.unlock();
  run_is_paused_.store(false);
  return true;
}

bool artdaq::EventBuilderCore::pause()
{
  mf::LogDebug("EventBuilderCore") << "Pausing run " << run_id_.run();
  pause_requested_.store(true);
  flush_mutex_.lock();

  bool endSucceeded = false;
  int attemptsToEnd = 1;
  endSucceeded = event_store_ptr_->endSubrun();
  while (! endSucceeded && attemptsToEnd < 3) {
    ++attemptsToEnd;
    mf::LogDebug("EventBuilderCore") << "Retrying EventStore::endSubrun()";
    endSucceeded = event_store_ptr_->endSubrun();
  }
  if (! endSucceeded) {
    mf::LogError("EventBuilderCore")
      << "EventStore::endSubrun in pause method failed after three tries.";
  }

  flush_mutex_.unlock();
  run_is_paused_.store(true);
  return true;
}

bool artdaq::EventBuilderCore::resume()
{
  mf::LogDebug("EventBuilderCore") << "Resuming run " << run_id_.run();
  eod_fragments_received_ = 0;
  pause_requested_.store(false);
  flush_mutex_.lock();
  event_store_ptr_->startSubrun();
  run_is_paused_.store(false);
  return true;
}

bool artdaq::EventBuilderCore::shutdown()
{
  /* We don't care about flushing data here.  The only way to transition to the
     shutdown state is from a state where there is no data taking.  All we have
     to do is signal the art input module that we're done taking data so that
     it can wrap up whatever it needs to do. */
  int readerReturnValue;
  bool endSucceeded = false;
  int attemptsToEnd = 1;
  endSucceeded = event_store_ptr_->endOfData(readerReturnValue);
  while (! endSucceeded && attemptsToEnd < 3) {
    ++attemptsToEnd;
    mf::LogDebug("EventBuilderCore") << "Retrying EventStore::endOfData()";
    endSucceeded = event_store_ptr_->endOfData(readerReturnValue);
  }
  return endSucceeded;
}

bool artdaq::EventBuilderCore::soft_initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("EventBuilderCore") << "soft_initialize method called with DAQ "
                               << "ParameterSet = \"" << pset.to_string()
                               << "\".";
  return true;
}

bool artdaq::EventBuilderCore::reinitialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("EventBuilderCore") << "reinitialize method called with DAQ "
                               << "ParameterSet = \"" << pset.to_string()
                               << "\".";
  return true;
}

size_t artdaq::EventBuilderCore::process_fragments()
{
  bool process_fragments = true;
  size_t senderSlot;
  std::vector<size_t> fragments_received(data_sender_count_ + first_data_sender_rank_, 0);
  std::vector<size_t> fragments_sent(data_sender_count_ + first_data_sender_rank_, 0);

  receiver_ptr_.reset(new artdaq::RHandles(mpi_buffer_count_,
                                           max_fragment_size_words_,
                                           data_sender_count_,
                                           first_data_sender_rank_));

  MPI_Barrier(local_group_comm_);

  mf::LogDebug("EventBuilderCore") << "Waiting for first fragment.";
  artdaq::MonitoredQuantity::TIME_POINT_T startTime;
  while (process_fragments) {
    artdaq::FragmentPtr pfragment(new artdaq::Fragment);

    size_t recvTimeout = inRunRecvTimeoutUSec_;
    if (stop_requested_.load()) {recvTimeout = endRunRecvTimeoutUSec_;}
    else if (pause_requested_.load()) {recvTimeout = pauseRunRecvTimeoutUSec_;}
    startTime = artdaq::MonitoredQuantity::getCurrentTime();
    senderSlot = receiver_ptr_->recvFragment(*pfragment, recvTimeout);
    statsHelper_.addSample(INPUT_WAIT_STAT_KEY,
                           (artdaq::MonitoredQuantity::getCurrentTime() - startTime));
    if (senderSlot == (size_t) MPI_ANY_SOURCE) {
      mf::LogInfo("EventBuilderCore")
        << "The receiving of data has stopped - ending the run.";
      event_store_ptr_->flushData();
      flush_mutex_.unlock();
      process_fragments = false;
      continue;
    }
    else if (senderSlot == artdaq::RHandles::RECV_TIMEOUT) {
      if (stop_requested_.load() &&
          recvTimeout == endRunRecvTimeoutUSec_) {
        mf::LogInfo("EventBuilderCore")
          << "Stop timeout expired - forcibly ending the run.";
	event_store_ptr_->flushData();
	flush_mutex_.unlock();
	process_fragments = false;
      }
      else if (pause_requested_.load() &&
               recvTimeout == pauseRunRecvTimeoutUSec_) {
        mf::LogInfo("EventBuilderCore")
          << "Pause timeout expired - forcibly pausing the run.";
	event_store_ptr_->flushData();
	flush_mutex_.unlock();
	process_fragments = false;
      }
      continue;
    }
    if (senderSlot >= fragments_received.size()) {
        mf::LogError("EventBuilderCore")
          << "Invalid senderSlot received from RHandles::recvFragment: "
          << senderSlot;
        continue;
    }
    fragments_received[senderSlot] += 1;
    if (artdaq::Fragment::isSystemFragmentType(pfragment->type())) {
      mf::LogDebug("EventBuilderCore")
        << "Sender slot = " << senderSlot
        << ", fragment type = " << ((int)pfragment->type())
        << ", sequence ID = " << pfragment->sequenceID();
    }

    ++fragment_count_in_run_;
    statsHelper_.addSample(INPUT_FRAGMENTS_STAT_KEY, pfragment->size());
    if (statsHelper_.readyToReport(INPUT_FRAGMENTS_STAT_KEY,
                                   fragment_count_in_run_)) {
      std::string statString = buildStatisticsString_();
      mf::LogDebug("EventBuilderCore") << statString;
      mf::LogDebug("EventBuilderCore")
        << "Received fragment " << fragment_count_in_run_
        << " with sequence id " << pfragment->sequenceID();
    }

    startTime = artdaq::MonitoredQuantity::getCurrentTime();
    if (pfragment->type() != artdaq::Fragment::EndOfDataFragmentType) {
      event_store_ptr_->insert(std::move(pfragment));
    } else {
      eod_fragments_received_++;
      /* We count the EOD fragment as a fragment received but the SHandles class
	 does not count it as a fragment sent which means we need to add one to
	 the total expected fragments. */
      fragments_sent[senderSlot] = *pfragment->dataBegin() + 1;
    }
    statsHelper_.addSample(STORE_EVENT_WAIT_STAT_KEY,
                           artdaq::MonitoredQuantity::getCurrentTime() - startTime);

    /* If we've received EOD fragments from all of the BoardReaders we can
       verify that we've also received every fragment that they have sent.  If
       all fragments are accounted for we can flush the EventStore, unlock the 
       mutex and exit out of this thread.*/
    if (eod_fragments_received_ == data_sender_count_) {
      bool fragmentsOutstanding = false;
      for (size_t i = 0; i < data_sender_count_ + first_data_sender_rank_; i++) {
	if (fragments_received[i] != fragments_sent[i]) {
	  fragmentsOutstanding = true;
	  break;
	}
      }

      if (!fragmentsOutstanding) {
	event_store_ptr_->flushData();
	flush_mutex_.unlock();
	process_fragments = false;
      }
    }
  }

  receiver_ptr_.reset(nullptr);
  return 0;
}

std::string artdaq::EventBuilderCore::report(std::string const&) const
{
  // lots of cool stuff that we can do here
  // - report on the number of fragments received and the number
  //   of events built (in the current or previous run
  // - report on the number of incomplete events in the EventStore
  //   (if running)
  std::string tmpString = "Event Builder run number = ";
  tmpString.append(boost::lexical_cast<std::string>(run_id_.run()));
  return tmpString;
}


std::string artdaq::EventBuilderCore::buildStatisticsString_()
{
  std::ostringstream oss;
  artdaq::MonitoredQuantityPtr mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(INPUT_FRAGMENTS_STAT_KEY);
  if (mqPtr.get() != 0) {
    //mqPtr->waitUntilAccumulatorsHaveBeenFlushed(3.0);
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    oss << "Input statistics: "
        << stats.recentSampleCount << " fragments received at "
        << stats.recentSampleRate  << " fragments/sec, date rate = "
        << (stats.recentValueRate * sizeof(artdaq::RawDataType)
            / 1024.0 / 1024.0) << " MB/sec, monitor window = "
        << stats.recentDuration << " sec, min::max event size = "
        << (stats.recentValueMin * sizeof(artdaq::RawDataType)
            / 1024.0 / 1024.0)
        << "::"
        << (stats.recentValueMax * sizeof(artdaq::RawDataType)
            / 1024.0 / 1024.0)
        << " MB" << std::endl;
    oss << "Average times per fragment: ";
    if (stats.recentSampleRate > 0.0) {
      oss << " elapsed time = "
          << (1.0 / stats.recentSampleRate) << " sec";
    }
  }

  mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(INPUT_WAIT_STAT_KEY);
  if (mqPtr.get() != 0) {
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    oss << ", input wait time = "
        << stats.recentValueAverage << " sec";
  }

  mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(STORE_EVENT_WAIT_STAT_KEY);
  if (mqPtr.get() != 0) {
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    oss << ", event store wait time = "
        << stats.recentValueAverage << " sec";
  }

  return oss.str();
}