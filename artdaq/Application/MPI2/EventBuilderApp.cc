#include "artdaq/Application/MPI2/EventBuilderApp.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Constructor.
 */
artdaq::EventBuilderApp::EventBuilderApp(int mpi_rank, MPI_Comm local_group_comm, std::string name) :
  mpi_rank_(mpi_rank), local_group_comm_(local_group_comm), name_(name)
{
}

// *******************************************************************
// *** The following methods implement the state machine operations.
// *******************************************************************

bool artdaq::EventBuilderApp::do_initialize(fhicl::ParameterSet const& pset, uint64_t, uint64_t)
{
  report_string_ = "";
  external_request_status_ = true;

  // in the following block, we first destroy the existing EventBuilder
  // instance, then create a new one.  Doing it in one step does not
  // produce the desired result since that creates a new instance and
  // then deletes the old one, and we need the opposite order.
  //event_builder_ptr_.reset(nullptr);
  if (event_builder_ptr_.get() == 0) {
    event_builder_ptr_.reset(new EventBuilderCore(mpi_rank_, local_group_comm_, name_));
    external_request_status_ = event_builder_ptr_->initialize(pset);
  }
  if (! external_request_status_) {
    report_string_ = "Error initializing an EventBuilderCore named";
    report_string_.append(name_ + " with ");
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }

  return external_request_status_;
}

bool artdaq::EventBuilderApp::do_start(art::RunID id, uint64_t, uint64_t)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->start(id);
  if (! external_request_status_) {
    report_string_ = "Error starting ";
    report_string_.append(name_ + " for run ");
    report_string_.append("number ");
    report_string_.append(boost::lexical_cast<std::string>(id.run()));
    report_string_.append(".");
  }

  event_building_future_ =
    std::async(std::launch::async, &EventBuilderCore::process_fragments,
               event_builder_ptr_.get());

  return external_request_status_;
}

bool artdaq::EventBuilderApp::do_stop(uint64_t, uint64_t)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->stop();
  if (! external_request_status_) {
    report_string_ = "Error stopping ";
    report_string_.append(name_ + ".");
  }

  if (event_building_future_.valid()) {
    event_building_future_.get();
  }
  return external_request_status_;
}

bool artdaq::EventBuilderApp::do_pause(uint64_t, uint64_t)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->pause();
  if (! external_request_status_) {
    report_string_ = "Error pausing ";
    report_string_.append(name_ + ".");
  }

  if (event_building_future_.valid()) {
    event_building_future_.get();
  }
  return external_request_status_;
}

bool artdaq::EventBuilderApp::do_resume(uint64_t, uint64_t)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->resume();
  if (! external_request_status_) {
    report_string_ = "Error resuming ";
    report_string_.append(name_ + ".");
  }

  event_building_future_ =
    std::async(std::launch::async, &EventBuilderCore::process_fragments,
               event_builder_ptr_.get());

  return external_request_status_;
}

bool artdaq::EventBuilderApp::do_shutdown(uint64_t )
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->shutdown();
  if (! external_request_status_) {
    report_string_ = "Error shutting down ";
    report_string_.append(name_ + ".");
  }
  return external_request_status_;
}

bool artdaq::EventBuilderApp::do_soft_initialize(fhicl::ParameterSet const& pset, uint64_t, uint64_t)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->soft_initialize(pset);
  if (! external_request_status_) {
    report_string_ = "Error soft-initializing ";
    report_string_.append(name_ + " with ");
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }
  return external_request_status_;
}

bool artdaq::EventBuilderApp::do_reinitialize(fhicl::ParameterSet const& pset, uint64_t, uint64_t)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->reinitialize(pset);
  if (! external_request_status_) {
    report_string_ = "Error reinitializing ";
    report_string_.append(name_ + " with ");
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }
  return external_request_status_;
}

void artdaq::EventBuilderApp::BootedEnter()
{
  mf::LogDebug(name_ + "App") << "Booted state entry action called.";

  // the destruction of any existing EventBuilderCore has to happen in the
  // Booted Entry action rather than the Initialized Exit action because the
  // Initialized Exit action is only called after the "init" transition guard
  // condition is executed.
  //event_builder_ptr_.reset(nullptr);
}

std::string artdaq::EventBuilderApp::report(std::string const& which) const
{
  // if there is an outstanding error, return that
  if (report_string_.length() > 0) {
    return report_string_;
  }

  // to-do: act differently depending on the value of "which"
  std::string tmpString = "Current state = " + status() + "\n";
  if (event_builder_ptr_.get() != 0) {
    tmpString.append(event_builder_ptr_->report(which));
  }
  return tmpString;
}
