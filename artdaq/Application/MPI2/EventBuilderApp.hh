#ifndef artdaq_Application_MPI2_EventBuilderApp_hh
#define artdaq_Application_MPI2_EventBuilderApp_hh

#include <future>

#include "artdaq/Application/Commandable.hh"
#include "artdaq/Application/MPI2/EventBuilderCore.hh"

namespace artdaq
{
  class EventBuilderApp;
}

class artdaq::EventBuilderApp : public artdaq::Commandable
{
public:
  EventBuilderApp(int mpi_rank, MPI_Comm local_group_comm);
  EventBuilderApp(EventBuilderApp const&) = delete;
  virtual ~EventBuilderApp() = default;
  EventBuilderApp& operator=(EventBuilderApp const&) = delete;

  // these methods provide the operations that are used by the state machine
  bool do_initialize(fhicl::ParameterSet const&) override;
  bool do_start(art::RunID run, uint64_t timeout, uint64_t timestamp) override;
  bool do_stop() override;
  bool do_pause() override;
  bool do_resume() override;
  bool do_shutdown() override;
  bool do_soft_initialize(fhicl::ParameterSet const&) override;
  bool do_reinitialize(fhicl::ParameterSet const&) override;

  void BootedEnter() override;

  /* Report_ptr */
  std::string report(std::string const&) const override;

private:
  int mpi_rank_;
  MPI_Comm local_group_comm_;
  std::unique_ptr<artdaq::EventBuilderCore> event_builder_ptr_;
  std::future<size_t> event_building_future_;
};

#endif /* artdaq_Application_MPI2_EventBuilderApp_hh */
