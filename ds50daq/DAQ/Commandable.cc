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
  externalRequestStatus_ = true;
  reportString_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.init(pset);
  if (externalRequestStatus_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after an init transition: "
      << initialState << " and " << finalState;
  }

  return (externalRequestStatus_);
}

/**
 * Processes the start request.
 */
bool ds50::Commandable::start(art::RunID id, std::string const& runtype)
{
  externalRequestStatus_ = true;
  reportString_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.start(id, runtype);
  if (externalRequestStatus_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a start transition: "
      << initialState << " and " << finalState;
  }

  return (externalRequestStatus_);
}

/**
 * Processes the stop request.
 */
bool ds50::Commandable::stop()
{
  externalRequestStatus_ = true;
  reportString_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.stop();
  if (externalRequestStatus_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a stop transition: "
      << initialState << " and " << finalState;
  }

  return (externalRequestStatus_);
}

/**
 * Processes the pause request.
 */
bool ds50::Commandable::pause()
{
  externalRequestStatus_ = true;
  reportString_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.pause();
  if (externalRequestStatus_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a pause transition: "
      << initialState << " and " << finalState;
  }

  return (externalRequestStatus_);
}

/**
 * Processes the resume request.
 */
bool ds50::Commandable::resume()
{
  externalRequestStatus_ = true;
  reportString_ = "All is OK.";

  std::string initialState = fsm_.getState().getName();
  fsm_.resume();
  if (externalRequestStatus_) {
    std::string finalState = fsm_.getState().getName();
    mf::LogDebug("CommandableInterface")
      << "States before and after a resume transition: "
      << initialState << " and " << finalState;
  }

  return (externalRequestStatus_);
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

bool ds50::Commandable::do_initialize(fhicl::ParameterSet const&)
{
  externalRequestStatus_ = true;
  return externalRequestStatus_;
}

bool ds50::Commandable::do_start(art::RunID, std::string const&)
{
  externalRequestStatus_ = true;
  return externalRequestStatus_;
}

bool ds50::Commandable::do_stop()
{
  externalRequestStatus_ = true;
  return externalRequestStatus_;
}

bool ds50::Commandable::do_pause()
{
  externalRequestStatus_ = true;
  return externalRequestStatus_;
}

bool ds50::Commandable::do_resume()
{
  externalRequestStatus_ = true;
  return externalRequestStatus_;
}

bool ds50::Commandable::do_reinitialize(fhicl::ParameterSet const&)
{
  externalRequestStatus_ = true;
  return externalRequestStatus_;
}

bool ds50::Commandable::do_softInitialize(fhicl::ParameterSet const&)
{
  externalRequestStatus_ = true;
  return externalRequestStatus_;
}

void ds50::Commandable::badTransition(const std::string& trans)
{
  reportString_ = "An invalid transition (";
  reportString_.append(trans);
  reportString_.append(") was requested.");

  mf::LogWarning("CommandableInterface") << reportString_;

  externalRequestStatus_ = false;
}
