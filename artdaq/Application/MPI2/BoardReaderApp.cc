#include "artdaq/Application/MPI2/BoardReaderApp.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

/**
 * Default constructor.
 */
artdaq::BoardReaderApp::BoardReaderApp(MPI_Comm local_group_comm, std::string name) :
  local_group_comm_(local_group_comm), name_(name)
{
}

// *******************************************************************
// *** The following methods implement the state machine operations.
// *******************************************************************

bool artdaq::BoardReaderApp::do_initialize(fhicl::ParameterSet const& pset, uint64_t timeout, uint64_t timestamp)
{
  report_string_ = "";
  external_request_status_ = true;

  // in the following block, we first destroy the existing BoardReader
  // instance, then create a new one.  Doing it in one step does not
  // produce the desired result since that creates a new instance and
  // then deletes the old one, and we need the opposite order.
  fragment_receiver_ptr_.reset(nullptr);
  fragment_receiver_ptr_.reset(new BoardReaderCore(local_group_comm_, name_));
  external_request_status_ = fragment_receiver_ptr_->initialize(pset, timeout, timestamp);
  if (! external_request_status_) {
    report_string_ = "Error initializing ";
    report_string_.append(name_ + " ");
    report_string_.append("with ParameterSet = \"" + pset.to_string() + "\".");
  }

  return external_request_status_;
}

bool artdaq::BoardReaderApp::do_start(art::RunID id, uint64_t timeout, uint64_t timestamp)
{
  report_string_ = "";
  external_request_status_ = fragment_receiver_ptr_->start(id, timeout, timestamp);
  if (! external_request_status_) {
    report_string_ = "Error starting ";
    report_string_.append(name_ + " ");
    report_string_.append("for run number ");
    report_string_.append(boost::lexical_cast<std::string>(id.run()));
    report_string_.append(", timeout ");
    report_string_.append(boost::lexical_cast<std::string>(timeout));
    report_string_.append(", timestamp ");
    report_string_.append(boost::lexical_cast<std::string>(timestamp));
    report_string_.append(".");
  }

  fragment_processing_future_ =
    std::async(std::launch::async, &BoardReaderCore::process_fragments,
               fragment_receiver_ptr_.get());

  return external_request_status_;
}

bool artdaq::BoardReaderApp::do_stop(uint64_t timeout, uint64_t timestamp)
{
  report_string_ = "";
  external_request_status_ = fragment_receiver_ptr_->stop(timeout, timestamp);
  if (! external_request_status_) {
    report_string_ = "Error stopping ";
    report_string_.append(name_ + ".");
    return false;
  }

  if (fragment_processing_future_.valid()) {
    int number_of_fragments_sent = fragment_processing_future_.get();
    mf::LogDebug(name_ + "App::do_stop(uint64_t, uint64_t)")
      << "Number of fragments sent = " << number_of_fragments_sent
      << ".";
  }

  return external_request_status_;
}

bool artdaq::BoardReaderApp::do_pause(uint64_t timeout, uint64_t timestamp)
{
  report_string_ = "";
  external_request_status_ = fragment_receiver_ptr_->pause(timeout, timestamp);
  if (! external_request_status_) {
    report_string_ = "Error pausing ";
    report_string_.append(name_ + ".");
  }

  if (fragment_processing_future_.valid()) {
    int number_of_fragments_sent = fragment_processing_future_.get();
    mf::LogDebug(name_+"App::do_pause(uint64_t, uint64_t)")
      << "Number of fragments sent = " << number_of_fragments_sent
      << ".";
  }

  return external_request_status_;
}

bool artdaq::BoardReaderApp::do_resume(uint64_t timeout, uint64_t timestamp)
{
  report_string_ = "";
  external_request_status_ = fragment_receiver_ptr_->resume(timeout, timestamp);
  if (! external_request_status_) {
    report_string_ = "Error resuming ";
    report_string_.append(name_ + ".");
  }

  fragment_processing_future_ =
    std::async(std::launch::async, &BoardReaderCore::process_fragments,
               fragment_receiver_ptr_.get());

  return external_request_status_;
}

bool artdaq::BoardReaderApp::do_shutdown(uint64_t timeout )
{
  report_string_ = "";
  external_request_status_ = fragment_receiver_ptr_->shutdown(timeout);
  if (! external_request_status_) {
    report_string_ = "Error shutting down ";
    report_string_.append(name_ + ".");
  }
  return external_request_status_;
}

bool artdaq::BoardReaderApp::do_soft_initialize(fhicl::ParameterSet const& pset, uint64_t timeout, uint64_t timestamp)
{
  report_string_ = "";
  external_request_status_ = fragment_receiver_ptr_->soft_initialize(pset, timeout, timestamp);
  if (! external_request_status_) {
    report_string_ = "Error soft-initializing ";
    report_string_.append(name_ + " ");
    report_string_.append("with ParameterSet = \"" + pset.to_string() + "\".");
  }
  return external_request_status_;
}

bool artdaq::BoardReaderApp::do_reinitialize(fhicl::ParameterSet const& pset, uint64_t timeout, uint64_t timestamp)
{
  external_request_status_ = fragment_receiver_ptr_->reinitialize(pset, timeout, timestamp);
  if (! external_request_status_) {
    report_string_ = "Error reinitializing ";
    report_string_.append(name_ + " ");
    report_string_.append("with ParameterSet = \"" + pset.to_string() + "\".");
  }
  return external_request_status_;
}

void artdaq::BoardReaderApp::BootedEnter()
{
  mf::LogDebug(name_ + "App") << "Booted state entry action called.";

  // the destruction of any existing BoardReaderCore has to happen in the
  // Booted Entry action rather than the Initialized Exit action because the
  // Initialized Exit action is only called after the "init" transition guard
  // condition is executed.
  fragment_receiver_ptr_.reset(nullptr);
}

std::string artdaq::BoardReaderApp::report(std::string const& which) const
{
  // if there is an outstanding error, return that
  if (report_string_.length() > 0) {
    return report_string_;
  }

  // to-do: act differently depending on the value of "which"
  std::string tmpString = "Current state = " + status() + "\n";
  if (fragment_receiver_ptr_.get() != 0) {
    tmpString.append(fragment_receiver_ptr_->report(which));
  }
  return tmpString;
}
