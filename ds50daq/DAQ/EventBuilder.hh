#ifndef ds50daq_DAQ_EventBuilder_hh
#define ds50daq_DAQ_EventBuilder_hh

#include <string>
#include <vector>

#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/RunID.h"
#include "artdaq/DAQrate/quiet_mpi.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"
#include "artdaq/DAQrate/RHandles.hh"

namespace ds50
{
  class EventBuilder;
}

class ds50::EventBuilder
{
public:
  EventBuilder(int mpi_rank);
  EventBuilder(EventBuilder const&) = delete;
  ~EventBuilder();
  EventBuilder& operator=(EventBuilder const&) = delete;

  bool initialize(fhicl::ParameterSet const&);
  bool start(art::RunID);
  bool stop();
  bool pause();
  bool resume();
  bool shutdown();
  bool soft_initialize(fhicl::ParameterSet const&);
  bool reinitialize(fhicl::ParameterSet const&);

  size_t process_fragments();

private:
  int mpi_rank_;
  bool local_group_defined_;
  MPI_Comm local_group_comm_;

  std::string init_string_;
  uint64_t max_fragment_size_words_;
  size_t mpi_buffer_count_;
  size_t first_data_sender_rank_;
  size_t data_sender_count_;
  bool use_art_;
  bool print_event_store_stats_;
  art::RunID run_id_;

  std::unique_ptr<artdaq::RHandles> receiver_ptr_;
};

#endif
