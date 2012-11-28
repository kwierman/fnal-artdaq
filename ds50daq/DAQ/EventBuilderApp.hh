#ifndef ds50daq_DAQ_EventBuilderApp_hh
#define ds50daq_DAQ_EventBuilderApp_hh

#include <future>

#include "ds50daq/DAQ/Commandable.hh"
#include "ds50daq/DAQ/EventBuilder.hh"

namespace ds50
{
  class EventBuilderApp;
}

class ds50::EventBuilderApp : public ds50::Commandable
{
public:
  EventBuilderApp();
  EventBuilderApp(EventBuilderApp const&) = delete;
  virtual ~EventBuilderApp() = default;
  EventBuilderApp& operator=(EventBuilderApp const&) = delete;

  // these methods provide the operations that are used by the state machine
  void BootedEnter() override;
  bool do_initialize(fhicl::ParameterSet const&) override;
  bool do_start(art::RunID) override;
  bool do_pause() override;
  bool do_resume() override;
  bool do_stop() override;

private:
  std::unique_ptr<ds50::EventBuilder> event_builder_ptr_;
  std::future<long> event_building_future_;
};

#endif
