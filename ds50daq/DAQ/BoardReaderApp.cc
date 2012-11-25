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

void ds50::BoardReaderApp::BootedEnter()
{
  mf::LogDebug("BoardReaderApp") << "Booted state entry action called.";

  // the destruction of any existing FragmentReceiver has to happen in the
  // Booted Entry action rather than the Initialized Exit action because the
  // Initialized Exit action is only called after the "init" transition guard
  // condition is executed.
  fragment_receiver_ptr_.reset(nullptr);
}

bool ds50::BoardReaderApp::do_initialize(fhicl::ParameterSet const& pset)
{
  external_request_status_ = true;

  // pull out the relevant part of the ParameterSet
  fhicl::ParameterSet daq_pset;
  if (! get_daq_pset(pset, daq_pset)) {
    report_string_ = "Unable to find the DAQ parameters in the initialization ";
    report_string_.append("ParameterSet: \"" + pset.to_string() + "\".");
    mf::LogError(" BoardReaderApp") << report_string_;
    external_request_status_ = false;
    return external_request_status_;
  }

  // in the following block, we first destroy the existing FragmentReceiver
  // instance, then create a new one.  Doing it in one step does not
  // produce the desired result since that creates and new instance and
  // then deletes the old one.
  fragment_receiver_ptr_.reset(nullptr);
  fragment_receiver_ptr_.reset(new FragmentReceiver());
  external_request_status_ = fragment_receiver_ptr_->initialize(daq_pset);
  if (! external_request_status_) {
    report_string_ = "Error initializing the FragmentReceiver with DAQ ";
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }

  return external_request_status_;
}

bool ds50::BoardReaderApp::do_start(art::RunID id, int max_events)
{
  external_request_status_ = fragment_receiver_ptr_->start(id, max_events);
  if (! external_request_status_) {
    report_string_ = "Error starting the FragmentReceiver for run ";
    report_string_.append("number ");
    report_string_.append(boost::lexical_cast<std::string>(id.run()));
    report_string_.append(".");
  }

  //fragment_processing_future_ =
  //  std::async(std::launch::async, &fragmentReceiverApp,
  //             *fragment_receiver_ptr_);

  worker_thread_ = new std::thread(std::bind(&FragmentReceiver::process_events,
                                             fragment_receiver_ptr_.get()));

  return external_request_status_;
}

bool ds50::BoardReaderApp::do_pause()
{
  return true;
}

bool ds50::BoardReaderApp::do_resume()
{
  return true;
}

bool ds50::BoardReaderApp::do_stop()
{
  external_request_status_ = fragment_receiver_ptr_->stop();
  if (! external_request_status_) {
    report_string_ = "Error stopping the FragmentReceiver.";
  }

  worker_thread_->join();
  delete worker_thread_;

  return external_request_status_;
}
