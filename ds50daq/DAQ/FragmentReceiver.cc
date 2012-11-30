#include "ds50daq/DAQ/Config.hh"
#include "ds50daq/DAQ/FragmentReceiver.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/makeFragmentGenerator.hh"
#include "art/Utilities/Exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Default constructor.
 */
ds50::FragmentReceiver::FragmentReceiver() :
  local_group_defined_(false), generator_ptr_(nullptr)
{
  mf::LogDebug("FragmentReceiver") << "Constructor";
}

/**
 * Destructor.
 */
ds50::FragmentReceiver::~FragmentReceiver()
{
  if (local_group_defined_) {
    MPI_Comm_free(&local_group_comm_);
  }
  mf::LogDebug("FragmentReceiver") << "Destructor";
}

/**
 * Processes the initialize request.
 */
bool ds50::FragmentReceiver::initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("FragmentReceiver") << "initialize method called with DAQ \""
                                   << "ParameterSet = " << pset.to_string()
                                   << "\".";

  // pull out the relevant part of the ParameterSet
  fhicl::ParameterSet fr_pset =
    pset.get<fhicl::ParameterSet>("fragment_receiver");

#if 0
  // set up an MPI communication group with other FragmentReceivers
  int status =
    MPI_Comm_split(MPI_COMM_WORLD, ds50::Config::FragmentReceiverTask, 0,
                   &local_group_comm_);
  if (status == MPI_SUCCESS) {
    local_group_defined_ = true;
    mf::LogDebug("FragmentReceiver")
      << "Successfully created local communicator with identifier 0x"
      << std::hex << local_group_comm_ << std::dec << ".";
  }
  else {
    mf::LogError("FragmentReceiver")
      << "Failed to create the local MPI communicator group for "
      << "FragmentReceivers, status code = " << status << ".";
    return false;
  }
#endif

  // create the requested FragmentGenerator
  std::string frag_gen_name = fr_pset.get<std::string>("generator", "");
  if (frag_gen_name.length() == 0) {
    mf::LogError("FragmentReceiver")
      << "No fragment generator (parameter name = \"generator\") was "
      << "specified in the fragment_receiver ParameterSet.  The "
      << "DAQ initialization PSet was \"" << pset.to_string() << "\".";
    return false;
  }

  std::unique_ptr<artdaq::FragmentGenerator> tmp_gen_ptr;
  try {
    tmp_gen_ptr = artdaq::makeFragmentGenerator(frag_gen_name, fr_pset);
  }
  catch (art::Exception& excpt) {
    mf::LogError("FragmentReceiver") << excpt;
    return false;
  }
  catch (...) {
    mf::LogError("FragmentReceiver") << "Unknown exception.";
    return false;
  }

  generator_ptr_.reset(nullptr);
  try {
    DS50FragmentGenerator* tmp_ds50gen_bareptr =
      dynamic_cast<DS50FragmentGenerator*>(tmp_gen_ptr.get());
    if (tmp_ds50gen_bareptr) {
      tmp_gen_ptr.release();
      generator_ptr_.reset(tmp_ds50gen_bareptr);
    }
  }
  catch (...) {}
  if (! generator_ptr_) {
    mf::LogError("FragmentReceiver")
      << "Error: The requested fragment generator type (" << frag_gen_name
      << ") is not a DS50FragmentGenerator, and only DS50FragmentGenerators "
      << "are currently supported.";
    return false;
  }

  // determine the data sending parameters
  try {mpi_buffer_count_ = pset.get<size_t>("event_building_buffer_count");}
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The event_building_buffer_count parameter was not specified "
      << "in the DAQ initialization PSet: \"" << pset.to_string() << "\".";
    return false;
  }
  try {
    max_fragment_size_words_ = pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \"" << pset.to_string() << "\".";
    return false;
  }
  try {first_evb_rank_ = fr_pset.get<size_t>("first_event_builder_rank");}
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The first_event_builder_rank parameter was not specified "
      << "in the fragment_receiver initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }
  try {evb_count_ = fr_pset.get<size_t>("event_builder_count");}
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The event_builder_count parameter was not specified "
      << "in the fragment_receiver initialization PSet: \"" << pset.to_string()
      << "\".";
    return false;
  }

  return true;
}

bool ds50::FragmentReceiver::start(art::RunID id)
{
  generator_ptr_->start(id.run());
  return true;
}

bool ds50::FragmentReceiver::pause()
{
  return true;
}

bool ds50::FragmentReceiver::resume()
{
  return true;
}

bool ds50::FragmentReceiver::stop()
{
  generator_ptr_->stop();
  return true;
}

size_t ds50::FragmentReceiver::process_fragments()
{
  size_t fragment_count = 0;

  // try-catch block here?

  sender_ptr_.reset(new artdaq::SHandles(mpi_buffer_count_,
                                         max_fragment_size_words_,
                                         evb_count_,
                                         first_evb_rank_));

  artdaq::FragmentPtrs frags;
  while (generator_ptr_->getNext(frags)) {
    for (auto & fragPtr : frags) {
      sender_ptr_->sendFragment(std::move(*fragPtr));
      mf::LogDebug("FragmentReceiver")
        << "Processing fragment " << fragment_count << ".";
      ++fragment_count;
    }
    frags.clear();
    //sleep(1);
  }
  //sender_ptr_->sendEODFrag(1, 1);
  //sender_ptr_->waitAll();

  sender_ptr_.reset(nullptr);

  return fragment_count;
}
