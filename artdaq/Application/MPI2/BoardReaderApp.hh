#ifndef artdaq_Application_MPI2_BoardReaderApp_hh
#define artdaq_Application_MPI2_BoardReaderApp_hh

#include <future>
#include <thread>

#include "artdaq/Application/Commandable.hh"
#include "artdaq/Application/MPI2/BoardReaderCore.hh"

namespace artdaq
{
  class BoardReaderApp;
}

class artdaq::BoardReaderApp : public artdaq::Commandable
{
public:
  BoardReaderApp(MPI_Comm local_group_comm);
  BoardReaderApp(BoardReaderApp const&) = delete;
  virtual ~BoardReaderApp() = default;
  BoardReaderApp& operator=(BoardReaderApp const&) = delete;

  // these methods provide the operations that are used by the state machine
  bool do_initialize(fhicl::ParameterSet const&) override;
  bool do_start(art::RunID) override;
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
  MPI_Comm local_group_comm_;
  std::unique_ptr<artdaq::BoardReaderCore> fragment_receiver_ptr_;
  std::future<size_t> fragment_processing_future_;
};

#endif /* artdaq_Application_MPI2_BoardReaderApp_hh */
