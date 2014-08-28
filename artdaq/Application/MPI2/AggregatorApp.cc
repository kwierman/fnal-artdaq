#include "artdaq/Application/MPI2/AggregatorApp.hh"
#include "artdaq/Application/MPI2/AggregatorCore.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "artdaq/DAQrate/RHandles.hh"
#include "artdaq/Application/TaskType.hh"

#include "art/Framework/Art/artapp.h"

#include <iostream>

artdaq::AggregatorApp::AggregatorApp(int mpi_rank, MPI_Comm local_group_comm) :
  mpi_rank_(mpi_rank), local_group_comm_(local_group_comm) { }

// *******************************************************************
// *** The following methods implement the state machine operations.
// *******************************************************************

bool artdaq::AggregatorApp::do_initialize(fhicl::ParameterSet const& pset)
{
  report_string_ = "";

  //aggregator_ptr_.reset(nullptr);
  if (aggregator_ptr_.get() == 0) {
    aggregator_ptr_.reset(new AggregatorCore(mpi_rank_, local_group_comm_));
  }
  external_request_status_ = aggregator_ptr_->initialize(pset);
  if (!external_request_status_) {
    report_string_ = "Error initializing the AggregatorCore with ";
    report_string_.append("ParameterSet = \"" + pset.to_string() + "\".");
  }

  return external_request_status_;
}

bool artdaq::AggregatorApp::do_start(art::RunID id, uint64_t timeout, uint64_t timestamp)
{
  report_string_ = "";
  external_request_status_ = aggregator_ptr_->start(id);
  if (!external_request_status_) {
    report_string_ = "Error starting the AggregatorCore for run ";
    report_string_.append("number ");
    report_string_.append(boost::lexical_cast<std::string>(id.run()));
    report_string_.append(", timeout ");
    report_string_.append(boost::lexical_cast<std::string>(timeout));
    report_string_.append(", timestamp ");
    report_string_.append(boost::lexical_cast<std::string>(timestamp));
    report_string_.append(".");
  }

  aggregator_future_ =
    std::async(std::launch::async, &AggregatorCore::process_fragments,
               aggregator_ptr_.get());

  return external_request_status_;

}

bool artdaq::AggregatorApp::do_stop()
{
  report_string_ = "";
  external_request_status_ = aggregator_ptr_->stop();
  if (!external_request_status_) {
    report_string_ = "Error stopping the AggregatorCore";
  }

  if (aggregator_future_.valid()) {
    aggregator_future_.get();
  }
  return external_request_status_;
}

bool artdaq::AggregatorApp::do_pause()
{
  report_string_ = "";
  external_request_status_ = aggregator_ptr_->pause();
  if (!external_request_status_) {
    report_string_ = "Error pausing the AggregatorCore";
  }

  if (aggregator_future_.valid()) {
    aggregator_future_.get();
  }
  return external_request_status_;
}

bool artdaq::AggregatorApp::do_resume()
{
  report_string_ = "";
  external_request_status_ = aggregator_ptr_->resume();
  if (!external_request_status_) {
    report_string_ = "Error resuming the AggregatorCore";
  }

  aggregator_future_ =
    std::async(std::launch::async, &AggregatorCore::process_fragments,
               aggregator_ptr_.get());

  return external_request_status_;
}

bool artdaq::AggregatorApp::do_shutdown()
{
  report_string_ = "";
  external_request_status_ = aggregator_ptr_->shutdown();
  if (!external_request_status_) {
    report_string_ = "Error shutting down the AggregatorCore";
  }

  return external_request_status_;
}

bool artdaq::AggregatorApp::do_soft_initialize(fhicl::ParameterSet const&)
{
  return true;
}

bool artdaq::AggregatorApp::do_reinitialize(fhicl::ParameterSet const&)
{
  return true;
}

std::string artdaq::AggregatorApp::report(std::string const& which) const
{
  // if there is an outstanding error, return that
  if (report_string_.length() > 0) {
    return report_string_;
  }

  if (which == "event_count" || which == "run_duration" || which == "file_size") {
    return aggregator_ptr_->report(which);
  }

  return "Unknown request: " + which;
}
