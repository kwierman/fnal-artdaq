#ifndef artdaq_Application_Commandable_hh
#define artdaq_Application_Commandable_hh

#include <string>
#include <vector>

#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/RunID.h"
#include "artdaq/Application/Commandable_sm.h"  // must be included after others

namespace artdaq
{
  class Commandable;
}

class artdaq::Commandable
{
public:
  Commandable();
  Commandable(Commandable const&) = delete;
  virtual ~Commandable() = default;
  Commandable& operator=(Commandable const&) = delete;

  // these methods define the externally available commands
  bool initialize(fhicl::ParameterSet const&);
  bool start(art::RunID id, uint64_t timestamp);
  bool stop();
  bool pause();
  bool resume();
  bool shutdown();
  bool soft_initialize(fhicl::ParameterSet const&);
  bool reinitialize(fhicl::ParameterSet const&);

  /* Report_ptr */
  virtual std::string report(std::string const&) const {
    return report_string_;
  }
  std::string status() const;
  virtual bool reset_stats(std::string const& which) {
    if (which=="fail") {
      return false;
    }
    else {
      return true;
    }
  }
  std::vector<std::string> legal_commands() const;

  // these methods provide the operations that are used by the state machine
  virtual bool do_initialize(fhicl::ParameterSet const&);
  virtual bool do_start(art::RunID, uint64_t);
  virtual bool do_stop();
  virtual bool do_pause();
  virtual bool do_resume();
  virtual bool do_shutdown();
  virtual bool do_reinitialize(fhicl::ParameterSet const&);
  virtual bool do_soft_initialize(fhicl::ParameterSet const&);
  virtual void badTransition(const std::string& );

  virtual void BootedEnter();
  virtual void InRunExit();

protected:
  CommandableContext fsm_;
  bool external_request_status_;
  std::string report_string_;
};

#endif /* artdaq_Application_Commandable_hh */
