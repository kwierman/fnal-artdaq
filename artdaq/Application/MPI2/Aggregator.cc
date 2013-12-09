#include "xmlrpc-c/client_simple.hpp"
#include "artdaq/Application/MPI2/Aggregator.hh"
#include "art/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq/DAQrate/EventStore.hh"
#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQrate/SimpleQueueReader.hh"
#include "artdaq/Utilities/SimpleLookupPolicy.h"
#include "artdaq/DAQdata/NetMonHeader.hh"
#include "artdaq/DAQdata/RawEvent.hh"

//#include <xmlrpc-c/base.hpp>
//#include <xmlrpc-c/registry.hpp>
//#include <xmlrpc-c/server_abyss.hpp>

#include <sstream>
#include <iomanip>

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
namespace BFS = boost::filesystem;

const std::string artdaq::Aggregator::INPUT_EVENTS_STAT_KEY("AggregatorInputEvents");
const std::string artdaq::Aggregator::INPUT_WAIT_STAT_KEY("AggregatorInputWaitTime");
const std::string artdaq::Aggregator::STORE_EVENT_WAIT_STAT_KEY("AggregatorStoreEventWaitTime");

/**
 * Constructor.
 */
// TODO - make global queue size configurable
artdaq::Aggregator::Aggregator(int mpi_rank, MPI_Comm local_group_comm) :
  mpi_rank_(mpi_rank), local_group_comm_(local_group_comm),
  art_initialized_(false),
  data_sender_count_(0), event_queue_(artdaq::getGlobalQueue(10)),
  stop_requested_(false), local_pause_requested_(false),
  system_pause_requested_(false), shm_segment_id_(-1), shm_ptr_(NULL)
{
  mf::LogDebug("Aggregator") << "Constructor";
  stats_helper_.addMonitoredQuantityName(INPUT_EVENTS_STAT_KEY);
  stats_helper_.addMonitoredQuantityName(INPUT_WAIT_STAT_KEY);
  stats_helper_.addMonitoredQuantityName(STORE_EVENT_WAIT_STAT_KEY);
}

/**
 * Destructor.
 */
artdaq::Aggregator::~Aggregator()
{
  mf::LogDebug("Aggregator") << "Destructor";
}

/**
 * Processes the initialize request.
 */
bool artdaq::Aggregator::initialize(fhicl::ParameterSet const& pset)
{
  init_string_ = pset.to_string();
  mf::LogDebug("Aggregator") << "initialize method called with DAQ "
                             << "ParameterSet = \"" << init_string_ << "\".";

  // pull out the relevant parts of the ParameterSet
  fhicl::ParameterSet daq_pset;
  try {
    daq_pset = pset.get<fhicl::ParameterSet>("daq");
  }
  catch (...) {
    mf::LogError("Aggregator")
      << "Unable to find the DAQ parameters in the initialization "
      << "ParameterSet: \"" + pset.to_string() + "\".";
    return false;
  }
  fhicl::ParameterSet agg_pset;
  try {
    agg_pset = daq_pset.get<fhicl::ParameterSet>("aggregator");
  }
  catch (...) {
    mf::LogError("Aggregator")
      << "Unable to find the aggregator parameters in the DAQ "
      << "initialization ParameterSet: \"" + daq_pset.to_string() + "\".";
    return false;
  }

  // determine the data receiver parameters
  try {
    max_fragment_size_words_ = daq_pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("Aggregator")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \""
      << daq_pset.to_string() << "\".";
    return false;
  }
  try {mpi_buffer_count_ = agg_pset.get<size_t>("mpi_buffer_count");}
  catch (...) {
    mf::LogError("Aggregator")
      << "The  mpi_buffer_count parameter was not specified "
      << "in the aggregator initialization PSet: \""
      << agg_pset.to_string() << "\".";
    return false;
  }
  try {
    first_data_sender_rank_ = agg_pset.get<size_t>("first_event_builder_rank");
  }
  catch (...) {
    mf::LogError("Aggregator")
      << "The first_event_builder_rank parameter was not specified "
      << "in the aggregator initialization PSet: \""
      << agg_pset.to_string() << "\".";
    return false;
  }
  try {data_sender_count_ = agg_pset.get<size_t>("event_builder_count");}
  catch (...) {
    mf::LogError("Aggregator")
      << "The event_builder_count parameter was not specified "
      << "in the aggregator initialization PSet: \""
      << agg_pset.to_string() << "\".";
    return false;
  }
  try {
    expected_events_per_bunch_ =
      agg_pset.get<size_t>("expected_events_per_bunch");}
  catch (...) {
    mf::LogError("Aggregator")
      << "The expected_events_per_bunch parameter was not specified "
      << "in the aggregator initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }

  is_data_logger_ = false;
  is_online_monitor_ = false;
  if (((size_t)mpi_rank_) == (first_data_sender_rank_ + data_sender_count_)) {
    is_data_logger_ = true;
  }
  if (((size_t)mpi_rank_) == (first_data_sender_rank_ + data_sender_count_ + 1)) {
    is_online_monitor_ = true;
  }
  mf::LogDebug("Aggregator") << "Rank " << mpi_rank_
                             << ", is_data_logger  = " << is_data_logger_
                             << ", is_online_monitor = " << is_online_monitor_;

  disk_writing_directory_ = "";
  try {
    fhicl::ParameterSet output_pset =
      pset.get<fhicl::ParameterSet>("outputs");
    fhicl::ParameterSet normalout_pset =
      output_pset.get<fhicl::ParameterSet>("normalOutput");
    std::string filename = normalout_pset.get<std::string>("fileName", "");
    if (filename.size() > 0) {
      size_t pos = filename.rfind("/");
      if (pos != std::string::npos) {
        disk_writing_directory_ = filename.substr(0, pos);
      }
    }
  }
  catch (...) {}

  std::string xmlrpcClientString =
    agg_pset.get<std::string>("xmlrpc_client_list", "");
  if (xmlrpcClientString.size() > 0) {
    xmlrpc_client_lists_.clear();
    boost::char_separator<char> sep1(";");
    boost::tokenizer<boost::char_separator<char>>
      primaryTokens(xmlrpcClientString, sep1);
    boost::tokenizer<boost::char_separator<char>>::iterator iter1;
    boost::tokenizer<boost::char_separator<char>>::iterator
      endIter1 = primaryTokens.end();
    for (iter1 = primaryTokens.begin(); iter1 != endIter1; ++iter1) {
      boost::char_separator<char> sep2(",");
      boost::tokenizer<boost::char_separator<char>>
        secondaryTokens(*iter1, sep2);
      boost::tokenizer<boost::char_separator<char>>::iterator iter2;
      boost::tokenizer<boost::char_separator<char>>::iterator
        endIter2 = secondaryTokens.end();
      int clientGroup = -1;
      std::string url = "";
      int loopCount = 0;
      for (iter2 = secondaryTokens.begin(); iter2 != endIter2; ++iter2) {
        switch (loopCount) {
        case 0:
          url = *iter2;
          break;
        case 1:
          try {
            clientGroup = boost::lexical_cast<int>(*iter2);
          }
          catch (...) {}
          break;
        default:
          mf::LogWarning("Aggregator")
            << "Unexpected XMLRPC client list element, index = "
            << loopCount << ", value = \"" << *iter2 << "\"";
        }
        ++loopCount;
      }
      if (clientGroup >= 0 && url.size() > 0) {
        int elementsNeeded = clientGroup+1-((int)xmlrpc_client_lists_.size());
        for (int idx = 0; idx < elementsNeeded; ++idx) {
          std::vector<std::string> tmpVec;
          xmlrpc_client_lists_.push_back(tmpVec);
        }
        xmlrpc_client_lists_[clientGroup].push_back(url);
      }
    }
  }
  double fileSizeMB = agg_pset.get<double>("file_size_MB", 0);
  file_close_threshold_bytes_ = ((size_t) fileSizeMB * 1024.0 * 1024.0);
  file_close_timeout_secs_ = agg_pset.get<time_t>("file_duration", 0);
  file_close_event_count_ = agg_pset.get<size_t>("file_event_count", 0);

  size_t event_queue_depth = agg_pset.get<size_t>("event_queue_depth", 20);
  double event_queue_wait_time = agg_pset.get<double>("event_queue_wait_time", 5.0);
  print_event_store_stats_ = agg_pset.get<bool>("print_event_store_stats", false);

  inrun_recv_timeout_usec_=agg_pset.get<size_t>("inrun_recv_timeout_usec",    100000);
  endrun_recv_timeout_usec_=agg_pset.get<size_t>("endrun_recv_timeout_usec",20000000);
  pause_recv_timeout_usec_=agg_pset.get<size_t>("endrun_recv_timeout_usec",3000000);

  onmon_event_prescale_ = agg_pset.get<size_t>("onmon_event_prescale", 1);

  // fetch the monitoring parameters and create the MonitoredQuantity instances
  stats_helper_.createCollectors(agg_pset, 100, 20.0, 60.0);

  if (event_store_ptr_ == nullptr) {
    artdaq::EventStore::ART_CFGSTRING_FCN * reader = &artapp_string_config;
    size_t desired_events_per_bunch = expected_events_per_bunch_;
    if (is_online_monitor_) {
      desired_events_per_bunch = 1;
    }
    event_store_ptr_.reset(new artdaq::EventStore(desired_events_per_bunch, 1,
						  mpi_rank_, init_string_,
						  reader, event_queue_depth, 
						  event_queue_wait_time, 
						  print_event_store_stats_));
    event_store_ptr_->setSeqIDModulus(desired_events_per_bunch);
    fhicl::ParameterSet tmp = pset;
    tmp.erase("daq");
    previous_pset_ = tmp;
  } else {
    fhicl::ParameterSet tmp = pset;
    tmp.erase("daq");
    if (tmp != previous_pset_) {
      mf::LogError("Aggregator")
        << "The art configuration can not be altered after art "
        << "has been configured.";
      return false;
    }
  }

  return true;
}

bool artdaq::Aggregator::start(art::RunID id)
{
  event_count_in_run_ = 0;
  event_count_in_subrun_ = 0;
  subrun_start_time_ = time(0);
  fragment_count_to_shm_ = 0;
  stats_helper_.resetStatistics();

  stop_requested_.store(false);
  local_pause_requested_.store(false);
  run_id_ = id;
  event_store_ptr_->startRun(run_id_.run());

  logMessage_("Started run " + boost::lexical_cast<std::string>(run_id_.run()));
  return true;
}

bool artdaq::Aggregator::stop()
{
  logMessage_("Stopping run " + boost::lexical_cast<std::string>(run_id_.run()) +
              ", " + boost::lexical_cast<std::string>(event_count_in_run_) +
              " events of all types received so far.");

  artdaq::MonitoredQuantityPtr mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(INPUT_EVENTS_STAT_KEY);
  if (mqPtr.get() != 0) {
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    std::ostringstream oss;
    oss << "Run " << run_id_.run() << " had an overall event rate of ";
    oss << std::fixed << std::setprecision(1) << stats.fullSampleRate;
    oss << " events/sec.";
    logMessage_(oss.str());
  }

  /* Nothing to do here.  The aggregator we clean up after itself once it has
     received all of the EOD fragments it expects.  Higher level code will block
     until the process_fragments() thread exits. */
  stop_requested_.store(true);
  return true;
}

bool artdaq::Aggregator::pause()
{
  logMessage_("Pausing run " + boost::lexical_cast<std::string>(run_id_.run()) +
              ", " + boost::lexical_cast<std::string>(event_count_in_run_) +
              " events of all types received so far.");

  /* Nothing to do here.  The aggregator we clean up after itself once it has
     received all of the EOD fragments it expects.  Higher level code will block
     until the process_fragments() thread exits. */
  local_pause_requested_.store(true);
  return true;
}

bool artdaq::Aggregator::resume()
{
  event_count_in_subrun_ = 0;
  subrun_start_time_ = time(0);
  local_pause_requested_.store(false);

  logMessage_("Resuming run " + boost::lexical_cast<std::string>(run_id_.run()));
  event_store_ptr_->startSubrun();
  return true;
}

bool artdaq::Aggregator::shutdown()
{
  int readerReturnValue;
  bool endSucceeded = false;
  int attemptsToEnd = 1;
  endSucceeded = event_store_ptr_->endOfData(readerReturnValue);
  while (! endSucceeded && attemptsToEnd < 3) {
    ++attemptsToEnd;
    endSucceeded = event_store_ptr_->endOfData(readerReturnValue);
  }
  return endSucceeded;
}

bool artdaq::Aggregator::soft_initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("Aggregator") << "soft_initialize method called with DAQ "
                             << "ParameterSet = \"" << pset.to_string()
                             << "\".";
  return true;
}

bool artdaq::Aggregator::reinitialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("Aggregator") << "reinitialize method called with DAQ "
                             << "ParameterSet = \"" << pset.to_string()
                             << "\".";
  return true;
}

size_t artdaq::Aggregator::process_fragments()
{
  size_t true_data_sender_count = data_sender_count_;
  if (is_online_monitor_) {
    true_data_sender_count = 1;
  }

  size_t eodFragmentsReceived = 0;
  bool process_fragments = true;
  size_t senderSlot;
  std::vector<size_t> fragments_received(true_data_sender_count + first_data_sender_rank_, 0);
  std::vector<size_t> fragments_sent(true_data_sender_count + first_data_sender_rank_, 0);
  artdaq::FragmentPtr endSubRunMsg(nullptr);
  bool eodWasCopied = false;
  bool esrWasCopied = false;

  if (is_data_logger_) {
    receiver_ptr_.reset(new artdaq::RHandles(mpi_buffer_count_,
                                             max_fragment_size_words_,
                                             true_data_sender_count,
                                             first_data_sender_rank_));
    attachToSharedMemory_(false);
  }
  else {
    attachToSharedMemory_(true);
  }

  mf::LogDebug("Aggregator") << "Waiting for first fragment.";
  artdaq::MonitoredQuantity::TIME_POINT_T startTime;
  while (process_fragments) {
    artdaq::FragmentPtr fragmentPtr(new artdaq::Fragment);

    size_t recvTimeout = inrun_recv_timeout_usec_;
    if (stop_requested_.load()) {recvTimeout = endrun_recv_timeout_usec_;}
    else if (local_pause_requested_.load()) {recvTimeout = pause_recv_timeout_usec_;}

    startTime = artdaq::MonitoredQuantity::getCurrentTime();
    if (is_data_logger_) {
      senderSlot = receiver_ptr_->recvFragment(*fragmentPtr, recvTimeout);
    }
    else if (is_online_monitor_) {
      senderSlot = receiveFragmentFromSharedMemory_(*fragmentPtr, recvTimeout);
    }
    else {
      usleep(recvTimeout);
      senderSlot = artdaq::RHandles::RECV_TIMEOUT;
    }
    stats_helper_.addSample(INPUT_WAIT_STAT_KEY,
                            (artdaq::MonitoredQuantity::getCurrentTime() - startTime));
    if (senderSlot == (size_t) MPI_ANY_SOURCE) {
      if (endSubRunMsg != nullptr) {
        mf::LogInfo("Aggregator")
          << "The receiving of data has stopped - ending the run.";
        event_store_ptr_->flushData();
        artdaq::RawEvent_ptr subRunEvent(new artdaq::RawEvent(run_id_.run(), 1, 0));
        subRunEvent->insertFragment(std::move(endSubRunMsg));
        daqrate::seconds const enq_timeout(5.0);
        bool enqStatus = event_queue_.enqTimedWait(subRunEvent, enq_timeout);
        if (! enqStatus) {
          mf::LogError("Aggregator") << "Failed to enqueue SubRun event.";
        }
      }
      else {
        mf::LogError("Aggregator")
          << "The receiving of data has stopped, but no endSubRun message "
          << "is available to send to art.";
      }
      process_fragments = false;
      continue;
    }
    else if (senderSlot == artdaq::RHandles::RECV_TIMEOUT) {
      if (stop_requested_.load() &&
          recvTimeout == endrun_recv_timeout_usec_) {
        if (endSubRunMsg != nullptr) {
          mf::LogInfo("Aggregator")
            << "Stop timeout expired - forcibly ending the run.";
          event_store_ptr_->flushData();
          artdaq::RawEvent_ptr subRunEvent(new artdaq::RawEvent(run_id_.run(), 1, 0));
          subRunEvent->insertFragment(std::move(endSubRunMsg));
          daqrate::seconds const enq_timeout(5.0);
          bool enqStatus = event_queue_.enqTimedWait(subRunEvent, enq_timeout);
          if (! enqStatus) {
            mf::LogError("Aggregator") << "Failed to enqueue SubRun event.";
          }
        }
        else {
          if (event_count_in_subrun_ > 0) {
            mf::LogError("Aggregator")
              << "Timeout receiving fragments after stop, but no endSubRun message "
              << "is available to send to art.";
          }
          else {
            std::string msg("Timeout receiving fragments after stop, but no ");
            msg.append("endSubRun message is available to send to art.");
            logMessage_(msg);
          }
        }
        process_fragments = false;
      }
      else if (local_pause_requested_.load() &&
               recvTimeout == pause_recv_timeout_usec_) {
        if (endSubRunMsg != nullptr) {
          mf::LogInfo("Aggregator")
            << "Pause timeout expired - forcibly pausing the run.";
          event_store_ptr_->flushData();
          artdaq::RawEvent_ptr subRunEvent(new artdaq::RawEvent(run_id_.run(), 1, 0));
          subRunEvent->insertFragment(std::move(endSubRunMsg));
          daqrate::seconds const enq_timeout(5.0);
          bool enqStatus = event_queue_.enqTimedWait(subRunEvent, enq_timeout);
          if (! enqStatus) {
            mf::LogError("Aggregator") << "Failed to enqueue SubRun event.";
          }
        }
        else {
          mf::LogError("Aggregator")
            << "Timeout receiving fragments after pause, but no endSubRun message "
            << "is available to send to art.";
        }
        process_fragments = false;
      }
      continue;
    }
    if (senderSlot >= fragments_received.size()) {
      mf::LogError("Aggregator")
        << "Invalid senderSlot received from RHandles::recvFragment: "
        << senderSlot;
      continue;
    }
    fragments_received[senderSlot] += 1;
    if (artdaq::Fragment::isSystemFragmentType(fragmentPtr->type()) &&
        fragmentPtr->type() != artdaq::Fragment::DataFragmentType) {
      mf::LogDebug("Aggregator")
        << "Sender slot = " << senderSlot
        << ", fragment type = " << ((int)fragmentPtr->type())
        << ", sequence ID = " << fragmentPtr->sequenceID();
    }

    // 11-Sep-2013, KAB - protect against invalid fragments
    if (fragmentPtr->type() == artdaq::Fragment::InvalidFragmentType) {
      size_t fragSize = fragmentPtr->size() * sizeof(artdaq::RawDataType);
      mf::LogError("Aggregator") << "Fragment received with type of "
                                 << "INVALID.  Size = " << fragSize
                                 << ", sequence ID = " << fragmentPtr->sequenceID()
                                 << ", fragment ID = " << fragmentPtr->fragmentID()
                                 << ", and type = " << ((int) fragmentPtr->type());
      continue;
    }

    ++event_count_in_run_;
    ++event_count_in_subrun_;
    if (event_count_in_run_ == 1) {
      logMessage_("Received event " +
                  boost::lexical_cast<std::string>(event_count_in_run_) +
                  " with sequence id " +
                  boost::lexical_cast<std::string>(fragmentPtr->sequenceID()) +
                  ".");
    }
    stats_helper_.addSample(INPUT_EVENTS_STAT_KEY, fragmentPtr->size());
    if (stats_helper_.readyToReport(INPUT_EVENTS_STAT_KEY,
                                    event_count_in_run_)) {
      std::string statString = buildStatisticsString_();
      logMessage_(statString);
      logMessage_("Received event " +
                  boost::lexical_cast<std::string>(event_count_in_run_) +
                  " with sequence id " +
                  boost::lexical_cast<std::string>(fragmentPtr->sequenceID()) +
                  " (run " +
                  boost::lexical_cast<std::string>(run_id_.run()) +
                  ").");
    }

    bool fragmentWasCopied = false;
    if (is_data_logger_ && (event_count_in_run_ % onmon_event_prescale_) == 0) {
      copyFragmentToSharedMemory_(fragmentWasCopied,
                                  esrWasCopied, eodWasCopied,
                                  *fragmentPtr, 0);
    }

    startTime = artdaq::MonitoredQuantity::getCurrentTime();
    if (!art_initialized_) {
      /* The init fragment should always be the first fragment out of the
         EventBuilder. */
      if (fragmentPtr->type() == artdaq::Fragment::InitFragmentType) {
        mf::LogDebug("Aggregator") << "Init";
        if (is_data_logger_) {
          copyFragmentToSharedMemory_(fragmentWasCopied,
                                      esrWasCopied, eodWasCopied,
                                      *fragmentPtr, 500000);
        }
        artdaq::RawEvent_ptr initEvent(new artdaq::RawEvent(run_id_.run(), 1, fragmentPtr->sequenceID()));
        initEvent->insertFragment(std::move(fragmentPtr));
        daqrate::seconds const enq_timeout(5.0);
        bool enqStatus = event_queue_.enqTimedWait(initEvent, enq_timeout);
        if (! enqStatus) {
          mf::LogError("Aggregator") << "Failed to enqueue INIT event.";
        }
        art_initialized_ = true;
      }
    } else {
      /* Note that in the currently implementation of the NetMon output/input
         modules there are no EndOfRun or Shutdown fragments. */
      if (fragmentPtr->type() == artdaq::Fragment::DataFragmentType) {
        if (is_data_logger_) {
          // soon, we will change this to use the insert() method that
          // gives us back the fragment if the event queue is full
          event_store_ptr_->insert(std::move(fragmentPtr));
        }
        else {
          event_store_ptr_->insert(std::move(fragmentPtr), false);
        }
      } else if (fragmentPtr->type() == artdaq::Fragment::EndOfSubrunFragmentType) {
        if (is_data_logger_) {
          copyFragmentToSharedMemory_(fragmentWasCopied,
                                      esrWasCopied, eodWasCopied,
                                      *fragmentPtr, 1000000);
        }
        /* We inject the EndSubrun fragment after all other data has been
           received.  The SHandles and RHandles classes do not guarantee that 
           data will be received in the same order it is sent.  We'll hold on to
           this fragment and inject it once we've received all EOD fragments. */
        endSubRunMsg = std::move(fragmentPtr);
      } else if (fragmentPtr->type() == artdaq::Fragment::EndOfDataFragmentType) {
        if (is_data_logger_) {
          copyFragmentToSharedMemory_(fragmentWasCopied,
                                      esrWasCopied, eodWasCopied,
                                      *fragmentPtr, 1000000);
        }
        eodFragmentsReceived++;
        /* We count the EOD fragment as a fragment received but the SHandles class
           does not count it as a fragment sent which means we need to add one to
           the total expected fragments. */
        fragments_sent[senderSlot] = *fragmentPtr->dataBegin() + 1;
      }
    }
    stats_helper_.addSample(STORE_EVENT_WAIT_STAT_KEY,
                            artdaq::MonitoredQuantity::getCurrentTime() - startTime);

    // 27-Sep-2013, KAB - added automatic file closing
    if (is_data_logger_ && disk_writing_directory_.size() > 0 &&
        ! stop_requested_.load() && ! system_pause_requested_.load() &&
        (event_count_in_run_ % 50) == 0) {
      time_t now = time(0);
      if ((now - subrun_start_time_) >= 30) {
        if ((file_close_event_count_ > 0 &&
             event_count_in_subrun_ >= file_close_event_count_) ||
            (file_close_timeout_secs_ > 0 &&
             (now - subrun_start_time_) >= file_close_timeout_secs_) ||
            (file_close_threshold_bytes_ > 0 &&
             getLatestFileSize_() >= file_close_threshold_bytes_)) {
          system_pause_requested_.store(true);
          if (pause_thread_.get() != 0) {
            pause_thread_->join();
          }
          pause_thread_.reset(new std::thread(&Aggregator::sendPauseAndResume_, this));

          // these should already have been done elsewhere, but just to be sure...
          event_count_in_subrun_ = 0;
          subrun_start_time_ = now;
        }
      }
    }

    /* If we've received EOD fragments from all of the EventBuilders we can
       verify that we've also received every fragment that they have sent.  If
       all fragments are accounted for we can flush the EventStoreand exit out 
       of this thread.*/
    if (eodFragmentsReceived >= true_data_sender_count && endSubRunMsg != nullptr) {
      bool fragmentsOutstanding = false;
      if (is_data_logger_) {
        for (size_t i = 0; i < true_data_sender_count + first_data_sender_rank_; i++) {
          if (fragments_received[i] != fragments_sent[i]) {
            fragmentsOutstanding = true;
            break;
          }
        }
      }

      if (!fragmentsOutstanding) {
        event_store_ptr_->flushData();
        artdaq::RawEvent_ptr subRunEvent(new artdaq::RawEvent(run_id_.run(), 1, 0));
        subRunEvent->insertFragment(std::move(endSubRunMsg));
        daqrate::seconds const enq_timeout(5.0);
        bool enqStatus = event_queue_.enqTimedWait(subRunEvent, enq_timeout);
        if (! enqStatus) {
          mf::LogError("Aggregator") << "Failed to enqueue SubRun event.";
        }
        process_fragments = false;
      }
    }
  }

  logMessage_("A subrun in run " +
              boost::lexical_cast<std::string>(run_id_.run()) +
              " has ended.  There have been " +
              boost::lexical_cast<std::string>(event_count_in_run_) +
              " events of all types so far in this run.");

  receiver_ptr_.reset(nullptr);
  if (is_online_monitor_) {
    detachFromSharedMemory_(true);
  }
  else {
    detachFromSharedMemory_(false);
  }
  return 0;
}

std::string artdaq::Aggregator::report(std::string const& which) const
{
  if (which == "event_count") {
    artdaq::MonitoredQuantityPtr mqPtr = artdaq::StatisticsCollection::getInstance().
      getMonitoredQuantity(INPUT_EVENTS_STAT_KEY);
    if (mqPtr.get() != 0) {
      artdaq::MonitoredQuantity::Stats stats;
      mqPtr->getStats(stats);
      return boost::lexical_cast<std::string>(stats.fullSampleCount);
    }
    else {
      return "-1";
    }
  }

  if (which == "run_duration") {
    artdaq::MonitoredQuantityPtr mqPtr = artdaq::StatisticsCollection::getInstance().
      getMonitoredQuantity(INPUT_EVENTS_STAT_KEY);
    if (mqPtr.get() != 0) {
      artdaq::MonitoredQuantity::Stats stats;
      mqPtr->getStats(stats);
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(1) << stats.fullDuration;
      return oss.str();
    }
    else {
      return "-1";
    }
  }

  if (which == "file_size") {
    size_t latestFileSize = getLatestFileSize_();
    return boost::lexical_cast<std::string>(latestFileSize);
  }

  // lots of cool stuff that we can do here
  // - report on the number of fragments received and the number
  //   of events built (in the current or previous run
  // - report on the number of incomplete events in the EventStore
  //   (if running)
  std::string tmpString = "Aggregator run number = ";
  tmpString.append(boost::lexical_cast<std::string>(run_id_.run()));
  return tmpString;
}

size_t artdaq::Aggregator::getLatestFileSize_() const
{
  if (disk_writing_directory_.size() == 0) {return 0;}
  BFS::path outputDir(disk_writing_directory_);
  BFS::directory_iterator endIter;

  std::time_t latestFileTime = 0;
  size_t latestFileSize = 0;
  if (BFS::exists(outputDir) && BFS::is_directory(outputDir)) {
    for (BFS::directory_iterator dirIter(outputDir); dirIter != endIter; ++dirIter) {
      BFS::path pathObj = dirIter->path();
      if (pathObj.filename().string().find("RootOutput") != std::string::npos &&
          pathObj.filename().string().find("root") != std::string::npos) {
        if (BFS::last_write_time(pathObj) >= latestFileTime) {
          latestFileTime = BFS::last_write_time(pathObj);
          latestFileSize = BFS::file_size(pathObj);
        }
      }
    }
  }
  time_t now = time(0);
  if ((now - latestFileTime) < 60) {
    return latestFileSize;
  }
  else {
    return 0;
  }
}

bool artdaq::Aggregator::sendPauseAndResume_()
{
  xmlrpc_c::clientSimple myClient;
  for (size_t igrp = 0; igrp < xmlrpc_client_lists_.size(); ++igrp) {
    for (size_t idx = 0; idx < xmlrpc_client_lists_[igrp].size(); ++idx) {
      //sleep(2);
      xmlrpc_c::value result;
      myClient.call((xmlrpc_client_lists_[igrp])[idx], "daq.pause", &result);
      std::string const resultString = xmlrpc_c::value_string(result);
      mf::LogDebug("Aggregator") << (xmlrpc_client_lists_[igrp])[idx]
                                 << " " << resultString;
    }
  }
  for (int igrp = (xmlrpc_client_lists_.size()-1); igrp >= 0; --igrp) {
    for (size_t idx = 0; idx < xmlrpc_client_lists_[igrp].size(); ++idx) {
      //sleep(2);
      xmlrpc_c::value result;
      myClient.call((xmlrpc_client_lists_[igrp])[idx], "daq.resume", &result);
      std::string const resultString = xmlrpc_c::value_string(result);
      mf::LogDebug("Aggregator") << (xmlrpc_client_lists_[igrp])[idx]
                                 << " " << resultString;
    }
  }
  system_pause_requested_.store(false);
  return true;
}

void artdaq::Aggregator::logMessage_(std::string const& text)
{
  if (is_data_logger_) {
    mf::LogInfo("Aggregator") << text;
  }
  else {
    mf::LogDebug("Aggregator") << text;
  }
}

std::string artdaq::Aggregator::buildStatisticsString_()
{
  std::ostringstream oss;
  artdaq::MonitoredQuantityPtr mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(INPUT_EVENTS_STAT_KEY);
  if (mqPtr.get() != 0) {
    //mqPtr->waitUntilAccumulatorsHaveBeenFlushed(3.0);
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    oss << "Input statistics: "
        << stats.recentSampleCount << " events received at "
        << stats.recentSampleRate  << " events/sec, date rate = "
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

void artdaq::Aggregator::attachToSharedMemory_(bool initialize)
{
  shm_segment_id_ = -1;
  shm_ptr_ = NULL;

  shm_segment_id_ =
    shmget(0x4f4d4f4e, (max_fragment_size_words_ * sizeof(artdaq::RawDataType)),
           IPC_CREAT | 0666);

  if (shm_segment_id_ > -1) {
    mf::LogDebug("Aggregator")
      << "Created/fetched shared memory segment with ID = " << shm_segment_id_
      << " and size " << (max_fragment_size_words_ * sizeof(artdaq::RawDataType))
      << " bytes";
    shm_ptr_ = (ShmStruct*) shmat(shm_segment_id_, 0, 0);
    if (shm_ptr_ != NULL) {
      if (initialize) {
        shm_ptr_->hasFragment = 0;
      }
      mf::LogDebug("Aggregator")
        << "Attached to shared memory segment at address 0x"
        << std::hex << shm_ptr_ << std::dec;
    }
    else {
      mf::LogError("Aggregator") << "Failed to attach to shared memory segment "
                                 << shm_segment_id_;
    }
  }
  else {
    mf::LogError("Aggregator") << "Failed to connect to shared memory segment"
                               << ", errno = " << errno << ".  Please check "
                               << "if a stale shared memory segment needs to "
                               << "be cleaned up. (ipcs, ipcrm -m <segId>)";
  }
}

void artdaq::Aggregator::
copyFragmentToSharedMemory_(bool& fragment_has_been_copied,
                            bool& esr_has_been_copied,
                            bool& eod_has_been_copied,
                            artdaq::Fragment& fragment,
                            size_t send_timeout_usec)
{
  // check if the fragment has already been copied to shared memory
  if (fragment_has_been_copied) {return;}

  // check if a fragment of this type has already been copied to shm
  size_t fragmentType = fragment.type();
  if (fragmentType == artdaq::Fragment::EndOfSubrunFragmentType &&
      esr_has_been_copied) {return;}
  if (fragmentType == artdaq::Fragment::EndOfDataFragmentType &&
      eod_has_been_copied) {return;}

  // verify that we have a shared memory segment
  if (shm_ptr_ == NULL) {return;}

  // wait for the shm to become free, if requested
  if (send_timeout_usec > 0) {
    size_t sleepTime = (send_timeout_usec / 10);
    int loopCount = 0;
    while (shm_ptr_->hasFragment == 1 && loopCount < 10) {
      if (fragmentType != artdaq::Fragment::DataFragmentType) {
        mf::LogDebug("Aggregator") << "Trying to copy fragment of type "
                                   << fragmentType
                                   << ", loopCount = "
                                   << loopCount;
      }
      usleep(sleepTime);
      ++loopCount;
    }
  }

  // copy the fragment if the shm is available
  if (shm_ptr_->hasFragment == 0) {
    artdaq::RawDataType* fragAddr = fragment.headerAddress();
    size_t fragSize = fragment.size() * sizeof(artdaq::RawDataType);

    // 10-Sep-2013, KAB - protect against large events and
    // invalid events (and large, invalid events)
    if (fragment.type() != artdaq::Fragment::InvalidFragmentType &&
        fragSize < ((max_fragment_size_words_ *
                     sizeof(artdaq::RawDataType)) -
                    sizeof(ShmStruct))) {
      memcpy(&shm_ptr_->fragmentInnards[0], fragAddr, fragSize);
      shm_ptr_->fragmentSizeWords = fragment.size();

      fragment_has_been_copied = true;
      if (fragmentType == artdaq::Fragment::EndOfSubrunFragmentType) {
        esr_has_been_copied = true;
      }
      if (fragmentType == artdaq::Fragment::EndOfDataFragmentType) {
        eod_has_been_copied = true;
      }

      shm_ptr_->hasFragment = 1;

      ++fragment_count_to_shm_;
      if ((fragment_count_to_shm_ % 250) == 0) {
        mf::LogDebug("Aggregator") << "Copied " << fragment_count_to_shm_
                                   << " fragments to shared memory in this run.";
      }
    }
    else {
      mf::LogWarning("Aggregator") << "Fragment invalid for shared memory! "
                                   << "fragment address and size = "
                                   << fragAddr << " " << fragSize << " "
                                   << "sequence ID, fragment ID, and type = "
                                   << fragment.sequenceID() << " "
                                   << fragment.fragmentID() << " "
                                   << ((int) fragment.type());
    }
  }
}

size_t artdaq::Aggregator::
receiveFragmentFromSharedMemory_(artdaq::Fragment& fragment,
                                 size_t receiveTimeout)
{
  if (shm_ptr_ != NULL) {
    int loopCount = 0;
    size_t sleepTime = receiveTimeout / 10;
    while (shm_ptr_->hasFragment == 0 && loopCount < 10) {
      usleep(sleepTime);
      ++loopCount;
    }
    if (shm_ptr_->hasFragment == 1) {
      fragment.resize(shm_ptr_->fragmentSizeWords);
      artdaq::RawDataType* fragAddr = fragment.headerAddress();
      size_t fragSize = fragment.size() * sizeof(artdaq::RawDataType);
      memcpy(fragAddr, &shm_ptr_->fragmentInnards[0], fragSize);
      shm_ptr_->hasFragment = 0;

      if (fragment.type() != artdaq::Fragment::DataFragmentType) {
        mf::LogDebug("Aggregator")
          << "Received fragment from shared memory, type ="
          << ((int)fragment.type()) << ", sequenceID = "
          << fragment.sequenceID();
      }

      return first_data_sender_rank_;
    }
    else {
      return artdaq::RHandles::RECV_TIMEOUT;
    }
  }
  else {
    usleep(receiveTimeout);
    return artdaq::RHandles::RECV_TIMEOUT;
  }
}

void artdaq::Aggregator::detachFromSharedMemory_(bool destroy)
{
  if (shm_ptr_ != NULL) {
    shmdt(shm_ptr_);
    shm_ptr_ = NULL;
  }
  if (destroy && shm_segment_id_ > -1) {
    shmctl(shm_segment_id_, IPC_RMID, NULL);
  }
}
