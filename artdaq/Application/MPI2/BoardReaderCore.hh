#ifndef artdaq_Application_MPI2_BoardReaderCore_hh
#define artdaq_Application_MPI2_BoardReaderCore_hh

#include <string>
#include <vector>
#include <iostream>

#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/RunID.h"
#include "artdaq/DAQrate/quiet_mpi.hh"
#include "artdaq/DAQrate/SHandles.hh"
#include "artdaq/Application/MPI2/StatisticsHelper.hh"

namespace artdaq
{
  class BoardReaderCore;
}

class artdaq::BoardReaderCore
{
public:
  static const std::string FRAGMENTS_PROCESSED_STAT_KEY;
  static const std::string INPUT_WAIT_STAT_KEY;
  static const std::string OUTPUT_WAIT_STAT_KEY;
  static const std::string FRAGMENTS_PER_READ_STAT_KEY;

  BoardReaderCore(MPI_Comm local_group_comm);
  BoardReaderCore(BoardReaderCore const&) = delete;
  ~BoardReaderCore();
  BoardReaderCore& operator=(BoardReaderCore const&) = delete;

  bool initialize(fhicl::ParameterSet const&);
  bool start(art::RunID);
  bool stop();
  bool pause();
  bool resume();
  bool shutdown();
  bool soft_initialize(fhicl::ParameterSet const&);
  bool reinitialize(fhicl::ParameterSet const&);

  size_t process_fragments();

  std::string report(std::string const&) const;

private:
  int mpi_rank_;
  MPI_Comm local_group_comm_;
  std::unique_ptr<CommandableFragmentGenerator> generator_ptr_;
  art::RunID run_id_;

  uint64_t max_fragment_size_words_;
  size_t mpi_buffer_count_;
  size_t first_evb_rank_;
  size_t evb_count_;
  int rt_priority_;
  bool skip_seqId_test_;
  bool synchronous_sends_;

  std::unique_ptr<artdaq::SHandles> sender_ptr_;

  size_t fragment_count_;
  artdaq::Fragment::sequence_id_t prev_seq_id_;

  // attributes and methods for statistics gathering & reporting
  artdaq::StatisticsHelper statsHelper_;
  std::string buildStatisticsString_();
};

#endif /* artdaq_Application_MPI2_BoardReaderCore_hh */
