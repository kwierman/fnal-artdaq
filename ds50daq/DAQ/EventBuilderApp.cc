#include "ds50daq/DAQ/EventBuilderApp.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Constructor.
 */
ds50::EventBuilderApp::EventBuilderApp(int mpi_rank) : mpi_rank_(mpi_rank)
{
}

// *******************************************************************
// *** The following methods implement the state machine operations.
// *******************************************************************

bool ds50::EventBuilderApp::do_initialize(fhicl::ParameterSet const& pset)
{
  report_string_ = "";
  external_request_status_ = true;

  // in the following block, we first destroy the existing EventBuilder
  // instance, then create a new one.  Doing it in one step does not
  // produce the desired result since that creates a new instance and
  // then deletes the old one, and we need the opposite order.
  event_builder_ptr_.reset(nullptr);
  event_builder_ptr_.reset(new EventBuilder(mpi_rank_));
  external_request_status_ = event_builder_ptr_->initialize(pset);
  if (! external_request_status_) {
    report_string_ = "Error initializing the EventBuilder with ";
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }

  return external_request_status_;
}

bool ds50::EventBuilderApp::do_start(art::RunID id)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->start(id);
  if (! external_request_status_) {
    report_string_ = "Error starting the EventBuilder for run ";
    report_string_.append("number ");
    report_string_.append(boost::lexical_cast<std::string>(id.run()));
    report_string_.append(".");
  }

  event_building_future_ =
    std::async(std::launch::async, &EventBuilder::process_fragments,
               event_builder_ptr_.get());

  return external_request_status_;
}

bool ds50::EventBuilderApp::do_stop()
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->stop();
  if (! external_request_status_) {
    report_string_ = "Error stopping the EventBuilder.";
  }

  event_building_future_.get();
  return external_request_status_;
}

bool ds50::EventBuilderApp::do_pause()
{
  std::cout << "ds50::EventBuilderApp::do_pause(): Called." << std::endl;
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->pause();
  if (! external_request_status_) {
    report_string_ = "Error pausing the EventBuilder.";
  }

  std::cout << "ds50::EventBuilderApp::do_pause(): Getting result from future." << std::endl;
  event_building_future_.get();
  std::cout << "ds50::EventBuilderApp::do_pause(): Returning" << std::endl;
  return external_request_status_;
}

bool ds50::EventBuilderApp::do_resume()
{
  std::cout << "ds50::EventBuilderApp::do_resume(): Called." << std::endl;
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->resume();
  if (! external_request_status_) {
    report_string_ = "Error resuming the EventBuilder.";
  }

  std::cout << "ds50::EventBuilderApp::do_resume(): Spawning thread." << std::endl;
  event_building_future_ =
    std::async(std::launch::async, &EventBuilder::process_fragments,
               event_builder_ptr_.get());

  std::cout << "ds50::EventBuilderApp::do_resume(): Returning." << std::endl;
  return external_request_status_;
}

bool ds50::EventBuilderApp::do_shutdown()
{
  std::cerr << "ds50::EventBuilderApp::do_shutdown(): Called." << std::endl;

  report_string_ = "";
  external_request_status_ = event_builder_ptr_->shutdown();
  if (! external_request_status_) {
    report_string_ = "Error shutting down the EventBuilder.";
  }
  return external_request_status_;
}

bool ds50::EventBuilderApp::do_soft_initialize(fhicl::ParameterSet const& pset)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->soft_initialize(pset);
  if (! external_request_status_) {
    report_string_ = "Error soft-initializing the EventBuilder with ";
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }
  return external_request_status_;
}

bool ds50::EventBuilderApp::do_reinitialize(fhicl::ParameterSet const& pset)
{
  report_string_ = "";
  external_request_status_ = event_builder_ptr_->reinitialize(pset);
  if (! external_request_status_) {
    report_string_ = "Error reinitializing the EventBuilder with ";
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }
  return external_request_status_;
}

void ds50::EventBuilderApp::BootedEnter()
{
  mf::LogDebug("EventBuilderApp") << "Booted state entry action called.";

  // the destruction of any existing EventBuilder has to happen in the
  // Booted Entry action rather than the Initialized Exit action because the
  // Initialized Exit action is only called after the "init" transition guard
  // condition is executed.
  //event_builder_ptr_.reset(nullptr);
}

std::string ds50::EventBuilderApp::report(std::string const& which) const
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
