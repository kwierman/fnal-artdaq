#include "artdaq/Application/Commandable.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Default constructor.
 */
artdaq::Commandable::Commandable() : fsm_(*this)
{
}

// **********************************************************************
// *** The following methods implement the externally available commands.
// **********************************************************************

/**
 * Processes the initialize request.
 */
bool artdaq::Commandable::initialize(fhicl::ParameterSet const& pset)
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.init(pset);
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after an init transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Processes the start request.
 */
bool artdaq::Commandable::start(art::RunID id)
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.start(id);
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a start transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Processes the stop request.
 */
bool artdaq::Commandable::stop()
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.stop();
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a stop transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Processes the pause request.
 */
bool artdaq::Commandable::pause()
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.pause();
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a pause transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Processes the resume request.
 */
bool artdaq::Commandable::resume()
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.resume();
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a resume transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Processes the shutdown request.
 */
bool artdaq::Commandable::shutdown()
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.shutdown();
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a shutdown transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Processes the soft_initialize request.
 */
bool artdaq::Commandable::soft_initialize(fhicl::ParameterSet const& pset)
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.soft_init(pset);
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after an soft_init transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Processes the reinitialize request.
 */
bool artdaq::Commandable::reinitialize(fhicl::ParameterSet const& pset)
{
  external_request_status_ = true;
  report_string_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.reinit(pset);
  if (external_request_status_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after an reinit transition: "
      << initialState << " and " << finalState;
  }

  return (external_request_status_);
}

/**
 * Returns the current state.
 */
std::string artdaq::Commandable::status() const
{
  std::string fullStateName = fsm_.getState().getName();
  size_t pos = fullStateName.rfind("::");
  if (pos != std::string::npos) {
    return fullStateName.substr(pos+2);
  }
  else {
    return fullStateName;
  }
}

/**
 * Returns the current list of legal commands.
 */
std::vector<std::string> artdaq::Commandable::legal_commands() const
{
  std::string currentState = this->status();
  if (currentState == "Ready") {
    return { "init", "soft_init", "start", "shutdown" };
  }
  if (currentState == "Running") {
    return { "pause", "stop", "init", "soft_init", "reinit", "shutdown" };
  }
  if (currentState == "Paused") {
    return { "resume", "stop", "init", "soft_init", "reinit", "shutdown" };
  }

  // Booted and Error
  return { "init", "shutdown" };
}

// *******************************************************************
// *** The following methods implement the state machine operations.
// *******************************************************************

bool artdaq::Commandable::do_initialize(fhicl::ParameterSet const&)
{
  mf::LogDebug("CommandableInterface") << "do_initialize called.";
  external_request_status_ = true;
  return external_request_status_;
}

bool artdaq::Commandable::do_start(art::RunID)
{
  mf::LogDebug("CommandableInterface") << "do_start called.";
  external_request_status_ = true;
  return external_request_status_;
}

bool artdaq::Commandable::do_stop()
{
  mf::LogDebug("CommandableInterface") << "do_stop called.";
  external_request_status_ = true;
  return external_request_status_;
}

bool artdaq::Commandable::do_pause()
{
  mf::LogDebug("CommandableInterface") << "do_pause called.";
  external_request_status_ = true;
  return external_request_status_;
}

bool artdaq::Commandable::do_resume()
{
  mf::LogDebug("CommandableInterface") << "do_resume called.";
  external_request_status_ = true;
  return external_request_status_;
}

bool artdaq::Commandable::do_shutdown()
{
  mf::LogDebug("CommandableInterface") << "do_shutdown called.";
  external_request_status_ = true;
  return external_request_status_;
}

bool artdaq::Commandable::do_reinitialize(fhicl::ParameterSet const&)
{
  mf::LogDebug("CommandableInterface") << "do_reinitialize called.";
  external_request_status_ = true;
  return external_request_status_;
}

bool artdaq::Commandable::do_soft_initialize(fhicl::ParameterSet const&)
{
  mf::LogDebug("CommandableInterface") << "do_soft_initialize called.";
  external_request_status_ = true;
  return external_request_status_;
}

void artdaq::Commandable::badTransition(const std::string& trans)
{
  report_string_ = "An invalid transition (";
  report_string_.append(trans);
  report_string_.append(") was requested.");

  mf::LogWarning("CommandableInterface") << report_string_;

  external_request_status_ = false;
}

void artdaq::Commandable::BootedEnter()
{
  mf::LogDebug("CommandableInterface") << "BootedEnter called.";
}

void artdaq::Commandable::InRunExit()
{
  mf::LogDebug("CommandableInterface") << "InRunExit called.";
}