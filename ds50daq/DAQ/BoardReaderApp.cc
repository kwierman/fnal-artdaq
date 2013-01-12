#include "ds50daq/DAQ/BoardReaderApp.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Default constructor.
 */
ds50::BoardReaderApp::BoardReaderApp()
{
}

// *******************************************************************
// *** The following methods implement the state machine operations.
// *******************************************************************

bool ds50::BoardReaderApp::do_initialize(fhicl::ParameterSet const& pset)
{
  external_request_status_ = true;

  // in the following block, we first destroy the existing FragmentReceiver
  // instance, then create a new one.  Doing it in one step does not
  // produce the desired result since that creates a new instance and
  // then deletes the old one, and we need the opposite order.
  fragment_receiver_ptr_.reset(nullptr);
  fragment_receiver_ptr_.reset(new FragmentReceiver());
  external_request_status_ = fragment_receiver_ptr_->initialize(pset);
  if (! external_request_status_) {
    report_string_ = "Error initializing the FragmentReceiver with ";
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }

  return external_request_status_;
}

bool ds50::BoardReaderApp::do_start(art::RunID id)
{
  external_request_status_ = fragment_receiver_ptr_->start(id);
  if (! external_request_status_) {
    report_string_ = "Error starting the FragmentReceiver for run ";
    report_string_.append("number ");
    report_string_.append(boost::lexical_cast<std::string>(id.run()));
    report_string_.append(".");
  }

  fragment_processing_future_ =
    std::async(std::launch::async, &FragmentReceiver::process_fragments,
               fragment_receiver_ptr_.get());

  return external_request_status_;
}

bool ds50::BoardReaderApp::do_stop()
{
  external_request_status_ = fragment_receiver_ptr_->stop();
  if (! external_request_status_) {
    report_string_ = "Error stopping the FragmentReceiver.";
    return false;
  }

  int number_of_fragments_sent = fragment_processing_future_.get();
  mf::LogDebug("BoardReaderApp::do_stop()")
    << "Number of fragments sent = " << number_of_fragments_sent
    << ".";

  return external_request_status_;
}

bool ds50::BoardReaderApp::do_pause()
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::BoardReaderApp::do_resume()
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::BoardReaderApp::do_shutdown()
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::BoardReaderApp::do_reinitialize(fhicl::ParameterSet const&)
{
  external_request_status_ = true;
  return external_request_status_;
}

bool ds50::BoardReaderApp::do_soft_initialize(fhicl::ParameterSet const&)
{
  external_request_status_ = true;
  return external_request_status_;
}

void ds50::BoardReaderApp::BootedEnter()
{
  mf::LogDebug("BoardReaderApp") << "Booted state entry action called.";

  // the destruction of any existing FragmentReceiver has to happen in the
  // Booted Entry action rather than the Initialized Exit action because the
  // Initialized Exit action is only called after the "init" transition guard
  // condition is executed.
  fragment_receiver_ptr_.reset(nullptr);
}
