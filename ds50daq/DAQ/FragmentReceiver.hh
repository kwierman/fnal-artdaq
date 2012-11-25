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
  bool start(art::RunID, int);
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

#if 0
namespace ds50
{
  int fragmentReceiverApp(FragmentReceiver& fr)
  {
    try {
      return fr.process_events();
    }
    catch (std::string const & msg) {
      std::cerr << "fragmentReceiverApp failed: "
                << msg;
      return -1;
    }
    catch (...) {
      return -1;
    }
  }
}
#endif

#endif
