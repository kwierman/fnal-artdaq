#ifndef ds50daq_DAQ_BoardReaderApp_hh
#define ds50daq_DAQ_BoardReaderApp_hh

#include <future>
#include <thread>

#include "ds50daq/DAQ/Commandable.hh"
#include "ds50daq/DAQ/FragmentReceiver.hh"

namespace ds50
{
  class BoardReaderApp;
}

class ds50::BoardReaderApp : public ds50::Commandable
{
public:
  BoardReaderApp();
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
  bool do_reinitialize(fhicl::ParameterSet const&) override;
  bool do_soft_initialize(fhicl::ParameterSet const&) override;

  void BootedEnter() override;

private:
  std::unique_ptr<ds50::FragmentReceiver> fragment_receiver_ptr_;
  std::future<size_t> fragment_processing_future_;
};

#endif
