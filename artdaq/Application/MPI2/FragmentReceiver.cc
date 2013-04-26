#include "artdaq/Application/TaskType.hh"
#include "artdaq/Application/MPI2/FragmentReceiver.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/makeFragmentGenerator.hh"
#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <pthread.h>
#include <sched.h>

/**
 * Default constructor.
 */
artdaq::FragmentReceiver::FragmentReceiver() :
  local_group_defined_(false), generator_ptr_(nullptr)
{
  mf::LogDebug("FragmentReceiver") << "Constructor";

  // set up an MPI communication group with other FragmentReceivers
  int status =
    MPI_Comm_split(MPI_COMM_WORLD, artdaq::TaskType::FragmentReceiverTask, 0,
                   &local_group_comm_);
  if (status == MPI_SUCCESS) {
    local_group_defined_ = true;
    MPI_Comm_rank(local_group_comm_, &mpi_rank_);

    mf::LogDebug("FragmentReceiver")
      << "Successfully created local communicator for type "
      << artdaq::TaskType::FragmentReceiverTask << ", identifier = 0x"
      << std::hex << local_group_comm_ << std::dec
      << ", rank = " << mpi_rank_ << ".";
  }
  else {
    mf::LogError("FragmentReceiver")
      << "Failed to create the local MPI communicator group for "
      << "FragmentReceivers, status code = " << status << ".";
  }
}

/**
 * Destructor.
 */
artdaq::FragmentReceiver::~FragmentReceiver()
{
  if (local_group_defined_) {
    MPI_Comm_free(&local_group_comm_);
  }
  mf::LogDebug("FragmentReceiver") << "Destructor";
}

/**
 * Processes the initialize request.
 */
bool artdaq::FragmentReceiver::initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("FragmentReceiver") << "initialize method called with "
                                   << "ParameterSet = \"" << pset.to_string()
                                   << "\".";

  // verify that the MPI group was set up successfully
  if (! local_group_defined_) {
    mf::LogError("FragmentReceiver")
      << "The necessary MPI group was not created in an earlier step, "
      << "and initialization can not proceed without that.";
    return false;
  }

  // pull out the relevant parts of the ParameterSet
  fhicl::ParameterSet daq_pset;
  try {
    daq_pset = pset.get<fhicl::ParameterSet>("daq");
  }
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "Unable to find the DAQ parameters in the initialization "
      << "ParameterSet: \"" + pset.to_string() + "\".";
    return false;
  }
  fhicl::ParameterSet fr_pset;
  try {
    fr_pset = daq_pset.get<fhicl::ParameterSet>("fragment_receiver");
  }
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "Unable to find the fragment_receiver parameters in the DAQ "
      << "initialization ParameterSet: \"" + daq_pset.to_string() + "\".";
    return false;
  }

  // create the requested FragmentGenerator
  std::string frag_gen_name = fr_pset.get<std::string>("generator", "");
  if (frag_gen_name.length() == 0) {
    mf::LogError("FragmentReceiver")
      << "No fragment generator (parameter name = \"generator\") was "
      << "specified in the fragment_receiver ParameterSet.  The "
      << "DAQ initialization PSet was \"" << daq_pset.to_string() << "\".";
    return false;
  }

  std::unique_ptr<artdaq::FragmentGenerator> tmp_gen_ptr;
  try {
    tmp_gen_ptr = artdaq::makeFragmentGenerator(frag_gen_name, fr_pset);
  }
  catch (art::Exception& excpt) {
    mf::LogError("FragmentReceiver")
      << "Exception creating a FragmentGenerator of type \""
      << frag_gen_name << "\" with parameter set \"" << fr_pset.to_string()
      << "\", exception = " << excpt;
    return false;
  }
  catch (cet::exception& excpt) {
    mf::LogError("FragmentReceiver")
      << "Exception creating a FragmentGenerator of type \""
      << frag_gen_name << "\" with parameter set \"" << fr_pset.to_string()
      << "\", exception = " << excpt;
    return false;
  }
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "Unknown exception creating a FragmentGenerator of type \""
      << frag_gen_name << "\" with parameter set \"" << fr_pset.to_string()
      << "\".";
    return false;
  }

  generator_ptr_.reset(nullptr);
  try {
    ds50::DS50FragmentGenerator* tmp_ds50gen_bareptr =
      dynamic_cast<ds50::DS50FragmentGenerator*>(tmp_gen_ptr.get());
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
  try {
    max_fragment_size_words_ = daq_pset.get<uint64_t>("max_fragment_size_words");
  }
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The max_fragment_size_words parameter was not specified "
      << "in the DAQ initialization PSet: \""
      << daq_pset.to_string() << "\".";
    return false;
  }
  try {mpi_buffer_count_ = fr_pset.get<size_t>("mpi_buffer_count");}
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The mpi_buffer_count parameter was not specified "
      << "in the fragment_receiver initialization PSet: \""
      << fr_pset.to_string() << "\".";
    return false;
  }
  try {first_evb_rank_ = fr_pset.get<size_t>("first_event_builder_rank");}
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The first_event_builder_rank parameter was not specified "
      << "in the fragment_receiver initialization PSet: \""
      << fr_pset.to_string() << "\".";
    return false;
  }
  try {evb_count_ = fr_pset.get<size_t>("event_builder_count");}
  catch (...) {
    mf::LogError("FragmentReceiver")
      << "The event_builder_count parameter was not specified "
      << "in the fragment_receiver initialization PSet: \""
      << fr_pset.to_string() << "\".";
    return false;
  }
  rt_priority_ = fr_pset.get<int>("rt_priority", 0);

  return true;
}

bool artdaq::FragmentReceiver::start(art::RunID id)
{
  generator_ptr_->start(id.run());
  return true;
}

bool artdaq::FragmentReceiver::stop()
{
  generator_ptr_->stop();
  return true;
}

bool artdaq::FragmentReceiver::pause()
{
  generator_ptr_->pause();
  return true;
}

bool artdaq::FragmentReceiver::resume()
{
  generator_ptr_->resume();
  return true;
}

bool artdaq::FragmentReceiver::shutdown()
{
  generator_ptr_.reset(nullptr);
  return true;
}

bool artdaq::FragmentReceiver::soft_initialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("FragmentReceiver") << "soft_initialize method called with "
                                   << "ParameterSet = \"" << pset.to_string()
                                   << "\".";
  return true;
}

bool artdaq::FragmentReceiver::reinitialize(fhicl::ParameterSet const& pset)
{
  mf::LogDebug("FragmentReceiver") << "reinitialize method called with "
                                   << "ParameterSet = \"" << pset.to_string()
                                   << "\".";
  return true;
}

size_t artdaq::FragmentReceiver::process_fragments()
{
  if (rt_priority_ > 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    sched_param s_param = {};
    s_param.sched_priority = rt_priority_;
    if (pthread_setschedparam(pthread_self(), SCHED_RR, &s_param))
      mf::LogWarning("FragmentReceiver") << "setting realtime prioriry failed";
#pragma GCC diagnostic pop
  }

  size_t fragment_count = 0;

  // try-catch block here?

  // how to turn RT PRI off?
  if (rt_priority_ > 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    sched_param s_param = {};
    s_param.sched_priority = rt_priority_;
    int status = pthread_setschedparam(pthread_self(), SCHED_RR, &s_param);
    if (status != 0) {
      mf::LogError("FragmentReceiver")
        << "Failed to set realtime priority to " << rt_priority_
        << ", return code = " << status;
    }
#pragma GCC diagnostic pop
  }

  sender_ptr_.reset(new artdaq::SHandles(mpi_buffer_count_,
                                         max_fragment_size_words_,
                                         evb_count_,
                                         first_evb_rank_));

  MPI_Barrier(local_group_comm_);

  mf::LogDebug("FragmentReceiver") << "Waiting for first fragment.";
  artdaq::Fragment::sequence_id_t prev_seq_id = 0;
  artdaq::FragmentPtrs frags;
  while (generator_ptr_->getNext(frags)) {
    for (auto & fragPtr : frags) {
      artdaq::Fragment::sequence_id_t sequence_id = fragPtr->sequenceID();
      if ((fragment_count % 250) == 0) {
        mf::LogDebug("FragmentReceiver")
          << "Sending fragment " << fragment_count
          << " with sequence id " << sequence_id << ".";
      }

      // check for continous sequence IDs
      if (abs(sequence_id-prev_seq_id) > 1) {
        mf::LogWarning("FragmentReceiver")
          << "Missing sequence IDs: current sequence ID = "
          << sequence_id << ", previous sequence ID = "
          << prev_seq_id << ".";
      }
      prev_seq_id = sequence_id;
      sender_ptr_->sendFragment(std::move(*fragPtr));
      ++fragment_count;
    }
    frags.clear();
  }

  // 07-Feb-2013, KAB
  // removing this barrier so that we can stop the trigger (V1495)
  // generation and readout before stopping the readout of the other cards
  //MPI_Barrier(local_group_comm_);

  sender_ptr_.reset(nullptr);
  return fragment_count;
}

std::string artdaq::FragmentReceiver::report(std::string const&) const
{
  return "Fragment receiver stats coming soon.";
}
