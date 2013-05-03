#include "artdaq/Application/TaskType.hh"
#include "artdaq/Application/MPI2/EventBuilder.hh"
#include "art/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq/DAQrate/EventStore.hh"
#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQrate/SimpleQueueReader.hh"
#include "artdaq/Utilities/SimpleLookupPolicy.h"
#include "artdaq/DAQdata/NetMonHeader.hh"

/**
 * Constructor.
 */
artdaq::EventBuilder::EventBuilder(int mpi_rank) :
  mpi_rank_(mpi_rank), local_group_defined_(false),
  data_sender_count_(0), art_initialized_(false)
{
  mf::LogDebug("EventBuilder") << "Constructor";

  // set up an MPI communication group with other EventBuilders
  int status =
    MPI_Comm_split(MPI_COMM_WORLD, artdaq::TaskType::EventBuilderTask, 0,
                   &local_group_comm_);
  if (status == MPI_SUCCESS) {
    local_group_defined_ = true;
    int temp_rank;
    MPI_Comm_rank(local_group_comm_, &temp_rank);
    mf::LogDebug("EventBuilder")
      << "Successfully created local communicator for type "
      << artdaq::TaskType::EventBuilderTask << ", identifier = 0x"
      << std::hex << local_group_comm_ << std::dec
      << ", rank = " << temp_rank << ".";
  }
  else {
    mf::LogError("EventBuilder")
      << "Failed to create the local MPI communicator group for "
      << "EventBuilders, status code = " << status << ".";
  }
}

/**
 * Destructor.
 */
artdaq::EventBuilder::~EventBuilder()
{
  if (local_group_defined_) {
    MPI_Comm_free(&local_group_comm_);
  }
  mf::LogDebug("EventBuilder") << "Destructor";
}

void artdaq::EventBuilder::initializeEventStore()
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
bool artdaq::EventBuilder::initialize(fhicl::ParameterSet const& pset)
{
  init_string_ = pset.to_string();
  mf::LogDebug("EventBuilder") << "initialize method called with DAQ "
                               << "ParameterSet = \"" << init_string_ << "\".";

  // pull out the relevant parts of the ParameterSet
  fhicl::ParameterSet daq_pset;
  try {
    daq_pset = pset.get<fhicl::ParameterSet>("daq");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "Unable to find the DAQ parameters in the initialization "
      << "ParameterSet: \"" + pset.to_string() + "\".";
    return false;
  }
  fhicl::ParameterSet evb_pset;
  try {
    evb_pset = daq_pset.get<fhicl::ParameterSet>("event_builder");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "Unable to find the event_builder parameters in the DAQ "
      << "initialization ParameterSet: \"" + daq_pset.to_string() + "\".";
    return false;
  }

  // verify that the MPI group was set up successfully
  if (! local_group_defined_) {
    mf::LogError("EventBuilder")
      << "The necessary MPI group was not created in an earlier step, "
      << "and initialization can not proceed without that.";
    return false;
  }

  // determine the data receiver parameters
  try {
    max_fragment_size_words_ = daq_pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \""
      << daq_pset.to_string() << "\".";
    return false;
  }
  try {mpi_buffer_count_ = evb_pset.get<size_t>("mpi_buffer_count");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The  mpi_buffer_count parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  try {
    first_data_sender_rank_ = evb_pset.get<size_t>("first_fragment_receiver_rank");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "The first_fragment_receiver_rank parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  try {data_sender_count_ = evb_pset.get<size_t>("fragment_receiver_count");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The fragment_receiver_count parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  try {
    expected_fragments_per_event_ =
      evb_pset.get<size_t>("expected_fragments_per_event");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The expected_fragments_per_event parameter was not specified "
      << "in the event_builder initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }

  // other parameters
  try {use_art_ = evb_pset.get<bool>("use_art");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The use_art parameter was not specified "
      << "in the event_builder initialization PSet: \""
      << evb_pset.to_string() << "\".";
    return false;
  }
  print_event_store_stats_ = evb_pset.get<bool>("print_event_store_stats", false);

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
      mf::LogError("EventBuilder")
	<< "The art configuration can not be altered after art "
	<< "has been configured.";
      return false;
    }
  }

  return true;
}

bool artdaq::EventBuilder::start(art::RunID id)
{
  run_id_ = id;
  eod_fragments_received_ = 0;
  flush_mutex_.lock();
  event_store_ptr_->startRun(id.run());
  return true;
}

bool artdaq::EventBuilder::stop()
{
  flush_mutex_.lock();
  event_store_ptr_->endSubrun();
  event_store_ptr_->endRun();
  flush_mutex_.unlock();
  return true;
}

bool artdaq::EventBuilder::pause()
{
  flush_mutex_.lock();
  event_store_ptr_->endSubrun();
  flush_mutex_.unlock();
  return true;
}

bool artdaq::EventBuilder::resume()
{
  eod_fragments_received_ = 0;
  flush_mutex_.lock();
  event_store_ptr_->startSubrun();
  return true;
}

bool artdaq::EventBuilder::shutdown()
{
  /* We don't care about flushing data here.  The only way to transition to the
     shutdown state is from a state where there is no data taking.  All we have
     to do is signal the art input module that we're done taking data so that
     it can wrap up whatever it needs to do. */
  event_store_ptr_->endOfData();
  return true;
}

bool artdaq::EventBuilder::soft_initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("EventBuilder") << "soft_initialize method called with DAQ "
                               << "ParameterSet = \"" << pset.to_string()
                               << "\".";
  return true;
}

bool artdaq::EventBuilder::reinitialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("EventBuilder") << "reinitialize method called with DAQ "
                               << "ParameterSet = \"" << pset.to_string()
                               << "\".";
  return true;
}

size_t artdaq::EventBuilder::process_fragments()
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

  mf::LogDebug("EventBuilder") << "Waiting for first fragment.";
  while (process_fragments) {
    artdaq::FragmentPtr pfragment(new artdaq::Fragment);
    senderSlot = receiver_ptr_->recvFragment(*pfragment);
    fragments_received[senderSlot] += 1;
    if (pfragment->type() != artdaq::Fragment::EndOfDataFragmentType) {
      event_store_ptr_->insert(std::move(pfragment));
    } else {
      eod_fragments_received_++;
      /* We count the EOD fragment as a fragment received but the SHandles class
	 does not count it as a fragment sent which means we need to add one to
	 the total expected fragments. */
      fragments_sent[senderSlot] = *pfragment->dataBegin() + 1;
    }
  
    /* If we've received EOD fragments from all of the FragmentReceivers we can
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

std::string artdaq::EventBuilder::report(std::string const&) const
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
