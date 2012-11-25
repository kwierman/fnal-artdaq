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
  EventBuilder();
  EventBuilder(EventBuilder const&) = delete;
  ~EventBuilder();
  EventBuilder& operator=(EventBuilder const&) = delete;

  bool initialize(fhicl::ParameterSet const&);
  bool start(art::RunID, int);
  bool pause();
  bool resume();
  bool stop();

private:
  bool local_group_defined_;
  MPI_Comm local_group_comm_;
  std::unique_ptr<artdaq::RHandles> receiver_ptr_;
};

#endif
