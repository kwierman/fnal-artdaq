#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

artdaq::CommandableFragmentGenerator::CommandableFragmentGenerator() :
  mutex_(),
  run_number_(-1), subrun_number_(-1),
  should_stop_(false), exception_(false),
  ev_counter_(1),
  board_id_(-1), fragment_id_(-1),
  sleep_on_stop_us_(0)
{
}


artdaq::CommandableFragmentGenerator::CommandableFragmentGenerator(const fhicl::ParameterSet &ps) :
  mutex_(),
  run_number_(-1), subrun_number_(-1),
  should_stop_(false), exception_(false),
  ev_counter_(1),
  board_id_(-1), fragment_id_(-1),
  sleep_on_stop_us_(0)
{
  board_id_ = ps.get<int> ("board_id");
  fragment_id_ = ps.get<int> ("fragment_id");

  sleep_on_stop_us_ = ps.get<int> ("sleep_on_stop_us", 0);
}

bool artdaq::CommandableFragmentGenerator::getNext(FragmentPtrs & output) {

  bool result = true;  
  
  if (should_stop() ) usleep( sleep_on_stop_us_);
  if (exception() ) return false;
  
  try { 
    std::lock_guard<std::mutex> lk(mutex_);
    result = getNext_( output );
  } catch (cet::exception &e) {
    mf::LogError ("getNext") << "exception caught: " << e;
    set_exception (true);
    return false;

  } catch (...) {
    mf::LogError ("getNext") << "unknown exception caught";
    set_exception (true);
    return false;
  }

  if ( ! result ) {
    mf::LogDebug("getNext") << "stopped ";
  }

  return result;

}

std::vector<artdaq::Fragment::fragment_id_t>
artdaq::CommandableFragmentGenerator::
fragmentIDs()
{
  return fragmentIDs_();
}

void artdaq::CommandableFragmentGenerator::StartCmd(int run) {

  if (run < 0) throw cet::exception("CommandableFragmentGenerator") << "negative run number";

  ev_counter_.store (1);
  should_stop_.store (false);
  exception_.store(false);
  run_number_ = run;
  subrun_number_ = 1;

  // no lock required: thread not started yet
  start();
}

void artdaq::CommandableFragmentGenerator::StopCmd() {
  should_stop_.store (true);
  std::unique_lock<std::mutex> lk(mutex_);

  stop();
}

void artdaq::CommandableFragmentGenerator::PauseCmd() {
  should_stop_.store (true);
  std::unique_lock<std::mutex> lk(mutex_);

  pause();
}

void artdaq::CommandableFragmentGenerator::ResumeCmd() {

  subrun_number_ += 1;
  should_stop_ = false; 

  // no lock required: thread not started yet
  resume();
}

std::string artdaq::CommandableFragmentGenerator::ReportCmd() 
{
  if (exception()) return "exception";
  std::lock_guard<std::mutex> lk(mutex_);

  return report();
}

