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

void ds50::EventBuilderApp::BootedEnter()
{
  mf::LogDebug("EventBuilderApp") << "Booted state entry action called.";

  // the destruction of any existing EventBuilder has to happen in the
  // Booted Entry action rather than the Initialized Exit action because the
  // Initialized Exit action is only called after the "init" transition guard
  // condition is executed.
  event_builder_ptr_.reset(nullptr);
}

bool ds50::EventBuilderApp::do_initialize(fhicl::ParameterSet const& pset)
{
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

bool ds50::EventBuilderApp::do_pause()
{
  return true;
}

bool ds50::EventBuilderApp::do_resume()
{
  return true;
}

bool ds50::EventBuilderApp::do_stop()
{
  external_request_status_ = event_builder_ptr_->stop();
  if (! external_request_status_) {
    report_string_ = "Error stopping the EventBuilder.";
  }

  int number_of_fragments_processed = event_building_future_.get();
  mf::LogDebug("EventBuilderApp::do_stop()")
    << "Number of fragments processed = " << number_of_fragments_processed
    << ".";

  return external_request_status_;
}
