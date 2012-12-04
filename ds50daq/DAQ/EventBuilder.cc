#include "ds50daq/DAQ/Config.hh"
#include "ds50daq/DAQ/EventBuilder.hh"
#include "art/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq/DAQrate/EventStore.hh"
#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQrate/SimpleQueueReader.hh"

/**
 * Constructor.
 */
ds50::EventBuilder::EventBuilder(int mpi_rank) : mpi_rank_(mpi_rank),
  local_group_defined_(false), data_sender_count_(0)
{
  mf::LogDebug("EventBuilder") << "Constructor";

  // set up an MPI communication group with other EventBuilders
  int status =
    MPI_Comm_split(MPI_COMM_WORLD, ds50::Config::EventBuilderTask, 0,
                   &local_group_comm_);
  if (status == MPI_SUCCESS) {
    local_group_defined_ = true;
    int temp_rank;
    MPI_Comm_rank(local_group_comm_, &temp_rank);
    mf::LogDebug("EventBuilder")
      << "Successfully created local communicator for type "
      << ds50::Config::EventBuilderTask << ", identifier = 0x"
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
ds50::EventBuilder::~EventBuilder()
{
  if (local_group_defined_) {
    MPI_Comm_free(&local_group_comm_);
  }
  mf::LogDebug("EventBuilder") << "Destructor";
}

/**
 * Processes the initialize request.
 */
bool ds50::EventBuilder::initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("EventBuilder") << "initialize method called with DAQ \""
                               << "ParameterSet = " << pset.to_string()
                               << "\".";

  // pull out the relevant part of the ParameterSet
  fhicl::ParameterSet evb_pset = pset.get<fhicl::ParameterSet>("event_builder");

  // verify that the MPI group was set up successfully
  if (! local_group_defined_) {
    mf::LogError("EventBuilder")
      << "The necessary MPI group was not created in an earlier step, "
      << "and initialization can not proceed without that.";
    return false;
  }

  // determine the data receiver parameters
  try {mpi_buffer_count_ = pset.get<size_t>("event_building_buffer_count");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The event_building_buffer_count parameter was not specified "
      << "in the DAQ initialization PSet: \"" << pset.to_string() << "\".";
    return false;
  }
  try {
    max_fragment_size_words_ = pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \"" << pset.to_string() << "\".";
    return false;
  }
  try {
    first_data_sender_rank_ = evb_pset.get<size_t>("first_fragment_receiver_rank");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "The first_fragment_receiver_rank parameter was not specified "
      << "in the event_builder initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }
  try {data_sender_count_ = evb_pset.get<size_t>("fragment_receiver_count");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The fragment_receiver_count parameter was not specified "
      << "in the event_builder initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }

  // other parameters
  try {use_art_ = evb_pset.get<bool>("useArt");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The useArt parameter was not specified "
      << "in the event_builder initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }
  print_event_store_stats_ = evb_pset.get<bool>("print_event_store_stats", false);

  return true;
}

bool ds50::EventBuilder::start(art::RunID id)
{
  run_id_ = id;
  return true;
}

bool ds50::EventBuilder::pause()
{
  return true;
}

bool ds50::EventBuilder::resume()
{
  return true;
}

bool ds50::EventBuilder::stop()
{
  return true;
}

size_t ds50::EventBuilder::process_fragments()
{
  size_t fragments_received = 0;
  size_t sources_sending = data_sender_count_;

  receiver_ptr_.reset(new artdaq::RHandles(mpi_buffer_count_,
                                           max_fragment_size_words_,
                                           data_sender_count_,
                                           first_data_sender_rank_));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
  char * dummyArgs[1] { "SimpleQueueReader" };
#pragma GCC diagnostic pop
  artdaq::EventStore::ARTFUL_FCN * reader = use_art_ ? &artapp :
    &artdaq::simpleQueueReaderApp;
  artdaq::EventStore events(data_sender_count_, run_id_.run(),
                            mpi_rank_, 1, dummyArgs,
                            //useArt ? conf_.art_argc_ : 1,
                            //useArt ? conf_.art_argv_ : dummyArgs,
                            reader, print_event_store_stats_);

  MPI_Barrier(local_group_comm_);

  mf::LogDebug("EventBuilder") << "Waiting for first fragment.";
  do {
    artdaq::FragmentPtr pfragment(new artdaq::Fragment);
    mf::LogDebug("EventBuilder") << "Before recvFragment call.";
    receiver_ptr_->recvFragment(*pfragment);
    mf::LogDebug("EventBuilder") << "After recvFragment call.";
    if (pfragment->type() == artdaq::Fragment::type_t::END_OF_DATA) {
      --sources_sending;
      mf::LogDebug("EventBuilder")
        << "Received END_OF_DATA fragment, " << sources_sending
        << " data senders still remain.";
    }
    else {
      mf::LogDebug("EventBuilder")
        << "Received fragment " << fragments_received << ".";
      ++fragments_received;
      events.insert(std::move(pfragment));
    }
  }
  while (sources_sending);

  //MPI_Barrier(local_group_comm_);

  receiver_ptr_.reset(nullptr);

  /*int rc =*/ events.endOfData();

  return fragments_received;
}
