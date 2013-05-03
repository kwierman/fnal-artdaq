#ifndef artdaq_Application_MPI2_FragmentReceiver_hh
#define artdaq_Application_MPI2_FragmentReceiver_hh

#include <string>
#include <vector>
#include <iostream>

#include "artdaq/DAQdata/FragmentGenerator.hh"
#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/RunID.h"
#include "artdaq/DAQrate/quiet_mpi.hh"
#include "artdaq/DAQrate/SHandles.hh"

namespace artdaq
{
  class FragmentReceiver;
}

class artdaq::FragmentReceiver
{
public:
  FragmentReceiver();
  FragmentReceiver(FragmentReceiver const&) = delete;
  ~FragmentReceiver();
  FragmentReceiver& operator=(FragmentReceiver const&) = delete;

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
  bool local_group_defined_;
  MPI_Comm local_group_comm_;
  std::unique_ptr<FragmentGenerator> generator_ptr_;

  uint64_t max_fragment_size_words_;
  size_t mpi_buffer_count_;
  size_t first_evb_rank_;
  size_t evb_count_;
  int rt_priority_;

  int mpi_rank_;

  std::unique_ptr<artdaq::SHandles> sender_ptr_;
};

#endif /* artdaq_Application_MPI2_FragmentReceiver_hh */
