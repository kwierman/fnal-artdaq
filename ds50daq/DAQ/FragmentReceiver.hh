#ifndef ds50daq_DAQ_FragmentReceiver_hh
#define ds50daq_DAQ_FragmentReceiver_hh

#include <string>
#include <vector>
#include <iostream>

#include "ds50daq/DAQ/DS50FragmentGenerator.hh"
#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/RunID.h"
#include "artdaq/DAQrate/quiet_mpi.hh"
#include "artdaq/DAQrate/SHandles.hh"

namespace ds50
{
  class FragmentReceiver;
}

class ds50::FragmentReceiver
{
public:
  FragmentReceiver();
  FragmentReceiver(FragmentReceiver const&) = delete;
  ~FragmentReceiver();
  FragmentReceiver& operator=(FragmentReceiver const&) = delete;

  bool initialize(fhicl::ParameterSet const&);
  bool start(art::RunID);
  bool pause();
  bool resume();
  bool stop();

  int process_events();

private:
  bool local_group_defined_;
  MPI_Comm local_group_comm_;
  std::unique_ptr<DS50FragmentGenerator> generator_ptr_;
  std::unique_ptr<artdaq::SHandles> sender_ptr_;
};

#endif
