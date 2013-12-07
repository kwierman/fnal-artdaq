#include "artdaq/DAQdata/FragmentGenerator.hh"

artdaq::FragmentGenerator::FragmentGenerator(const fhicl::ParameterSet &ps) :
  run_number_(-1), subrun_number_(-1)
{
  board_id_ = ps.get<int> ("board_id");
  fragment_id_ = ps.get<int> ("fragment_id");
}

bool artdaq::FragmentGenerator::getNext(FragmentPtrs & output) {
  return getNext_(output);
}

std::vector<artdaq::Fragment::fragment_id_t>
artdaq::FragmentGenerator::
fragmentIDs()
{
  return fragmentIDs_();
}

bool artdaq::FragmentGenerator::requiresStateMachine() const {
  return requiresStateMachine_();
}

void artdaq::FragmentGenerator::StartCmd(int run) {
  // thread not started yet

  if (run < 0) throw cet::exception("FragmentGenerator") << "negative run number";

  ev_counter_.store (1);
  should_stop_.store (false);
  exception_.store (false);
  run_number_ = run;
  subrun_number_ = 1;

  start();
}

void artdaq::FragmentGenerator::StopCmd() {
  should_stop_.store (true);
  std::unique_lock<std::mutex> lk(mutex_);

  stop();
}

void artdaq::FragmentGenerator::PauseCmd() {
  should_stop_.store (true);
  std::unique_lock<std::mutex> lk(mutex_);

  pause();
}

void artdaq::FragmentGenerator::ResumeCmd() {
  // no lock required: thread not started yet
  subrun_number_ += 1;
  should_stop_ = false; 
  resume();
}

std::string artdaq::FragmentGenerator::ReportCmd() 
{
  if (exception_.load ()) return "exception";
  std::lock_guard<std::mutex> lk(mutex_);

  return report();
}

