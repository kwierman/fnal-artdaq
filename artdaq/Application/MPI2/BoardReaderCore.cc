#include "artdaq/Application/TaskType.hh"
#include "artdaq/Application/MPI2/BoardReaderCore.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/Application/makeCommandableFragmentGenerator.hh"
#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <pthread.h>
#include <sched.h>
#include <algorithm>

const std::string artdaq::BoardReaderCore::
  FRAGMENTS_PROCESSED_STAT_KEY("BoardReaderCoreFragmentsProcessed");
const std::string artdaq::BoardReaderCore::
  INPUT_WAIT_STAT_KEY("BoardReaderCoreInputWaitTime");
const std::string artdaq::BoardReaderCore::
  OUTPUT_WAIT_STAT_KEY("BoardReaderCoreOutputWaitTime");
const std::string artdaq::BoardReaderCore::
  FRAGMENTS_PER_READ_STAT_KEY("BoardReaderCoreFragmentsPerRead");

/**
 * Default constructor.
 */
artdaq::BoardReaderCore::BoardReaderCore(MPI_Comm local_group_comm) :
  local_group_comm_(local_group_comm), generator_ptr_(nullptr)
{
  mf::LogDebug("BoardReaderCore") << "Constructor";
  statsHelper_.addMonitoredQuantityName(FRAGMENTS_PROCESSED_STAT_KEY);
  statsHelper_.addMonitoredQuantityName(INPUT_WAIT_STAT_KEY);
  statsHelper_.addMonitoredQuantityName(OUTPUT_WAIT_STAT_KEY);
  statsHelper_.addMonitoredQuantityName(FRAGMENTS_PER_READ_STAT_KEY);
}

/**
 * Destructor.
 */
artdaq::BoardReaderCore::~BoardReaderCore()
{
  mf::LogDebug("BoardReaderCore") << "Destructor";
}

/**
 * Processes the initialize request.
 */
bool artdaq::BoardReaderCore::initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("BoardReaderCore") << "initialize method called with "
                                   << "ParameterSet = \"" << pset.to_string()
                                   << "\".";

  // pull out the relevant parts of the ParameterSet
  fhicl::ParameterSet daq_pset;
  try {
    daq_pset = pset.get<fhicl::ParameterSet>("daq");
  }
  catch (...) {
    mf::LogError("BoardReaderCore")
      << "Unable to find the DAQ parameters in the initialization "
      << "ParameterSet: \"" + pset.to_string() + "\".";
    return false;
  }
  fhicl::ParameterSet fr_pset;
  try {
    fr_pset = daq_pset.get<fhicl::ParameterSet>("fragment_receiver");
  }
  catch (...) {
    mf::LogError("BoardReaderCore")
      << "Unable to find the fragment_receiver parameters in the DAQ "
      << "initialization ParameterSet: \"" + daq_pset.to_string() + "\".";
    return false;
  }

  // create the requested CommandableFragmentGenerator
  std::string frag_gen_name = fr_pset.get<std::string>("generator", "");
  if (frag_gen_name.length() == 0) {
    mf::LogError("BoardReaderCore")
      << "No fragment generator (parameter name = \"generator\") was "
      << "specified in the fragment_receiver ParameterSet.  The "
      << "DAQ initialization PSet was \"" << daq_pset.to_string() << "\".";
    return false;
  }

  try {
    generator_ptr_ = artdaq::makeCommandableFragmentGenerator(frag_gen_name, fr_pset);
  }
  catch (art::Exception& excpt) {
    mf::LogError("BoardReaderCore")
      << "Exception creating a CommandableFragmentGenerator of type \""
      << frag_gen_name << "\" with parameter set \"" << fr_pset.to_string()
      << "\", exception = " << excpt;
    return false;
  }
  catch (cet::exception& excpt) {
    mf::LogError("BoardReaderCore")
      << "Exception creating a CommandableFragmentGenerator of type \""
      << frag_gen_name << "\" with parameter set \"" << fr_pset.to_string()
      << "\", exception = " << excpt;
    return false;
  }
  catch (...) {
    mf::LogError("BoardReaderCore")
      << "Unknown exception creating a CommandableFragmentGenerator of type \""
      << frag_gen_name << "\" with parameter set \"" << fr_pset.to_string()
      << "\".";
    return false;
  }

  // determine the data sending parameters
  try {
    max_fragment_size_words_ = daq_pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("BoardReaderCore")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \""
      << daq_pset.to_string() << "\".";
    return false;
  }
  try {mpi_buffer_count_ = fr_pset.get<size_t>("mpi_buffer_count");}
  catch (...) {
    mf::LogError("BoardReaderCore")
      << "The mpi_buffer_count parameter was not specified "
      << "in the fragment_receiver initialization PSet: \""
      << fr_pset.to_string() << "\".";
    return false;
  }
  try {first_evb_rank_ = fr_pset.get<size_t>("first_event_builder_rank");}
  catch (...) {
    mf::LogError("BoardReaderCore")
      << "The first_event_builder_rank parameter was not specified "
      << "in the fragment_receiver initialization PSet: \""
      << fr_pset.to_string() << "\".";
    return false;
  }
  try {evb_count_ = fr_pset.get<size_t>("event_builder_count");}
  catch (...) {
    mf::LogError("BoardReaderCore")
      << "The event_builder_count parameter was not specified "
      << "in the fragment_receiver initialization PSet: \""
      << fr_pset.to_string() << "\".";
    return false;
  }
  rt_priority_ = fr_pset.get<int>("rt_priority", 0);

  // fetch the monitoring parameters and create the MonitoredQuantity instances
  statsHelper_.createCollectors(fr_pset, 100, 30.0, 180.0);

  // check if we should skip the sequence ID test...
  skip_seqId_test_ = (generator_ptr_->fragmentIDs().size() > 1);

  return true;
}

bool artdaq::BoardReaderCore::start(art::RunID id)
{
  fragment_count_ = 0;
  prev_seq_id_ = 0;
  statsHelper_.resetStatistics();

  generator_ptr_->StartCmd(id.run());
  run_id_ = id;

  mf::LogDebug("BoardReaderCore") << "Started run " << run_id_.run();
  return true;
}

bool artdaq::BoardReaderCore::stop()
{
  mf::LogDebug("BoardReaderCore") << "Stopping run " << run_id_.run()
                                   << " after " << fragment_count_
                                   << " fragments.";
  generator_ptr_->StopCmd();
  return true;
}

bool artdaq::BoardReaderCore::pause()
{
  mf::LogDebug("BoardReaderCore") << "Pausing run " << run_id_.run()
                                   << " after " << fragment_count_
                                   << " fragments.";
  generator_ptr_->PauseCmd();
  return true;
}

bool artdaq::BoardReaderCore::resume()
{
  mf::LogDebug("BoardReaderCore") << "Resuming run " << run_id_.run();
  generator_ptr_->ResumeCmd();
  return true;
}

bool artdaq::BoardReaderCore::shutdown()
{
  generator_ptr_.reset(nullptr);
  return true;
}

bool artdaq::BoardReaderCore::soft_initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("BoardReaderCore") << "soft_initialize method called with "
                                   << "ParameterSet = \"" << pset.to_string()
                                   << "\".";
  return true;
}

bool artdaq::BoardReaderCore::reinitialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("BoardReaderCore") << "reinitialize method called with "
                                   << "ParameterSet = \"" << pset.to_string()
                                   << "\".";
  return true;
}

size_t artdaq::BoardReaderCore::process_fragments()
{
  if (rt_priority_ > 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    sched_param s_param = {};
    s_param.sched_priority = rt_priority_;
    if (pthread_setschedparam(pthread_self(), SCHED_RR, &s_param))
      mf::LogWarning("BoardReaderCore") << "setting realtime prioriry failed";
#pragma GCC diagnostic pop
  }

  // try-catch block here?

  // how to turn RT PRI off?
  if (rt_priority_ > 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    sched_param s_param = {};
    s_param.sched_priority = rt_priority_;
    int status = pthread_setschedparam(pthread_self(), SCHED_RR, &s_param);
    if (status != 0) {
      mf::LogError("BoardReaderCore")
        << "Failed to set realtime priority to " << rt_priority_
        << ", return code = " << status;
    }
#pragma GCC diagnostic pop
  }

  sender_ptr_.reset(new artdaq::SHandles(mpi_buffer_count_,
                                         max_fragment_size_words_,
                                         evb_count_,
                                         first_evb_rank_,
                                         false));

  MPI_Barrier(local_group_comm_);

  mf::LogDebug("BoardReaderCore") << "Waiting for first fragment.";
  artdaq::MonitoredQuantity::TIME_POINT_T startTime;
  artdaq::FragmentPtrs frags;
  bool active = true;
  while (active) {
    startTime = artdaq::MonitoredQuantity::getCurrentTime();
    active = generator_ptr_->getNext(frags);
    statsHelper_.addSample(INPUT_WAIT_STAT_KEY,
                           artdaq::MonitoredQuantity::getCurrentTime() - startTime);
    if (! active) {break;}
    statsHelper_.addSample(FRAGMENTS_PER_READ_STAT_KEY, frags.size());

    startTime = artdaq::MonitoredQuantity::getCurrentTime();
    for (auto & fragPtr : frags) {
      artdaq::Fragment::sequence_id_t sequence_id = fragPtr->sequenceID();
      statsHelper_.addSample(FRAGMENTS_PROCESSED_STAT_KEY, fragPtr->size());

      if ((fragment_count_ % 250) == 0) {
        mf::LogDebug("BoardReaderCore")
          << "Sending fragment " << fragment_count_
          << " with sequence id " << sequence_id << ".";
      }

      // check for continous sequence IDs
      if (! skip_seqId_test_ && abs(sequence_id-prev_seq_id_) > 1) {
        mf::LogWarning("BoardReaderCore")
          << "Missing sequence IDs: current sequence ID = "
          << sequence_id << ", previous sequence ID = "
          << prev_seq_id_ << ".";
      }
      prev_seq_id_ = sequence_id;

      sender_ptr_->sendFragment(std::move(*fragPtr));
      ++fragment_count_;
      bool readyToReport =
        statsHelper_.readyToReport(FRAGMENTS_PROCESSED_STAT_KEY,
                                   fragment_count_);
      if (readyToReport) {
        std::string statString = buildStatisticsString_();
        mf::LogDebug("BoardReaderCore") << statString;
      }
      if (fragment_count_ == 1 || readyToReport) {
        mf::LogDebug("BoardReaderCore")
          << "Sending fragment " << fragment_count_
          << " with sequence id " << sequence_id << ".";
      }
    }
    statsHelper_.addSample(OUTPUT_WAIT_STAT_KEY,
                           artdaq::MonitoredQuantity::getCurrentTime() - startTime);
    frags.clear();
  }

  // 07-Feb-2013, KAB
  // removing this barrier so that we can stop the trigger (V1495)
  // generation and readout before stopping the readout of the other cards
  //MPI_Barrier(local_group_comm_);

  sender_ptr_.reset(nullptr);
  return fragment_count_;
}

std::string artdaq::BoardReaderCore::report(std::string const&) const
{
  return generator_ptr_->ReportCmd();
}

std::string artdaq::BoardReaderCore::buildStatisticsString_()
{
  std::ostringstream oss;
  double fragmentCount = 1.0;
  artdaq::MonitoredQuantityPtr mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(FRAGMENTS_PROCESSED_STAT_KEY);
  if (mqPtr.get() != 0) {
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    oss << "Fragment statistics: "
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
    fragmentCount = std::max(double(stats.recentSampleCount), 1.0);
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
        << (stats.recentValueSum / fragmentCount) << " sec";
  }

  mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(OUTPUT_WAIT_STAT_KEY);
  if (mqPtr.get() != 0) {
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    oss << ", output wait time = "
        << (stats.recentValueSum / fragmentCount) << " sec";
  }

  oss << std::endl << "Fragments per read: ";
  mqPtr = artdaq::StatisticsCollection::getInstance().
    getMonitoredQuantity(FRAGMENTS_PER_READ_STAT_KEY);
  if (mqPtr.get() != 0) {
    artdaq::MonitoredQuantity::Stats stats;
    mqPtr->getStats(stats);
    oss << "average = "
        << stats.recentValueAverage
        << ", min::max = "
        << stats.recentValueMin
        << "::"
        << stats.recentValueMax;
  }

  return oss.str();
}
