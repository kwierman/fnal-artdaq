#include "artdaq/DAQdata/FragmentGenerator.hh"

bool artdaq::FragmentGenerator::getNext(FragmentPtrs & output) {
  return getNext_(output);
}

std::vector<artdaq::Fragment::fragment_id_t>
artdaq::FragmentGenerator::
fragmentIDs()
{
  return fragmentIDs_();
}

int artdaq::FragmentGenerator::run_number() const {
  return run_number_;
}

int artdaq::FragmentGenerator::subrun_number() const {
  return subrun_number_;
}

bool artdaq::FragmentGenerator::requiresStateMachine() const {
  return requiresStateMachine_();
}

void artdaq::FragmentGenerator::start(int run) {
  run_number_ = run;
  subrun_number_ = 1;
  start_();
}

void artdaq::FragmentGenerator::stop() {
  stop_();
}

void artdaq::FragmentGenerator::pause() {
  pause_();
}

void artdaq::FragmentGenerator::resume() {
  subrun_number_ += 1;
  resume_();
}

std::string artdaq::FragmentGenerator::report() 
{
  return "Fragment generator stats not implemented.";
}
