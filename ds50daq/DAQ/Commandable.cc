#include "ds50daq/DAQ/Commandable.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Default constructor.
 */
ds50::Commandable::Commandable() : fsm_(*this)
{
}

// **********************************************************************
// *** The following methods implement the externally available commands.
// **********************************************************************

/**
 * Processes the initialize request.
 */
bool ds50::Commandable::initialize(fhicl::ParameterSet const& pset)
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
bool ds50::Commandable::start(art::RunID id)
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
bool ds50::Commandable::stop()
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
bool ds50::Commandable::pause()
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
bool ds50::Commandable::resume()
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
 * Returns the current state.
 */
std::string ds50::Commandable::status() const
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
std::vector<std::string> ds50::Commandable::legalCommands() const
{
  std::string currentState = this->status();
  if (currentState == "Ready") {
    return { "init", "start" };
  }
  if (currentState == "Running") {
    return { "init", "stop", "pause" };
  }
  if (currentState == "Paused") {
    return { "resume", "stop", "init" };
  }

  return { "init" };
}

// *******************************************************************
// *** The following methods implement the state machine operations.
// *******************************************************************

void ds50::Commandable::BootedEnter()
{
}

bool ds50::Commandable::do_initialize(fhicl::ParameterSet const&)
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::Commandable::do_start(art::RunID)
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::Commandable::do_pause()
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::Commandable::do_resume()
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::Commandable::do_stop()
{
  external_request_status_ = true;
  return external_request_status_;
}

void ds50::Commandable::InRunExit()
{
  mf::LogDebug("CommandableInterface") << "InRunExit called.";
}

bool ds50::Commandable::do_reinitialize(fhicl::ParameterSet const&)
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::Commandable::do_softInitialize(fhicl::ParameterSet const&)
{
  external_request_status_ = true;
  return external_request_status_;
}

void ds50::Commandable::badTransition(const std::string& trans)
{
  report_string_ = "An invalid transition (";
  report_string_.append(trans);
  report_string_.append(") was requested.");

  mf::LogWarning("CommandableInterface") << report_string_;

  external_request_status_ = false;
}
