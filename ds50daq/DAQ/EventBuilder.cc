#include "ds50daq/DAQ/Config.hh"
#include "ds50daq/DAQ/EventBuilder.hh"
#include "art/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Default constructor.
 */
ds50::EventBuilder::EventBuilder() : local_group_defined_(false)
{
  mf::LogDebug("EventBuilder") << "Constructor";
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

  // set up an MPI communication group with other EventBuilders
  int status =
    MPI_Comm_split(MPI_COMM_WORLD, ds50::Config::EventBuilderTask, 0,
                   &local_group_comm_);
  if (status == MPI_SUCCESS) {
    local_group_defined_ = true;
    mf::LogDebug("EventBuilder")
      << "Successfully created local communicator with identifier 0x"
      << std::hex << local_group_comm_ << std::dec << ".";
  }
  else {
    mf::LogError("EventBuilder")
      << "Failed to create the local MPI communicator group for "
      << "EventBuilders, status code = " << status << ".";
    return false;
  }

  // create the data receiver
  size_t mpi_buffer_count;
  try {mpi_buffer_count = pset.get<size_t>("event_building_buffer_count");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The event_building_buffer_count parameter was not specified "
      << "in the DAQ initialization PSet: \"" << pset.to_string() << "\".";
    return false;
  }
  uint64_t max_fragment_size_words;
  try {
    max_fragment_size_words = pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \"" << pset.to_string() << "\".";
    return false;
  }
  size_t first_frag_rec_rank;
  try {
    first_frag_rec_rank = evb_pset.get<size_t>("first_fragment_receiver_rank");
  }
  catch (...) {
    mf::LogError("EventBuilder")
      << "The first_fragment_receiver_rank parameter was not specified "
      << "in the event_builder initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }
  size_t frag_rec_count;
  try {frag_rec_count = evb_pset.get<size_t>("fragment_receiver_count");}
  catch (...) {
    mf::LogError("EventBuilder")
      << "The fragment_receiver_count parameter was not specified "
      << "in the event_builder initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }
  receiver_ptr_.reset(new artdaq::RHandles(mpi_buffer_count,
                                           max_fragment_size_words,
                                           frag_rec_count,
                                           first_frag_rec_rank));
  return true;
}

bool ds50::EventBuilder::start(art::RunID /*id*/)
{
  //generator_ptr_->start(id, max_events);
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
  //generator_ptr_->stop();
  return true;
}
