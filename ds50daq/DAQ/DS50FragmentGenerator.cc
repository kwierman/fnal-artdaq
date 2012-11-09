#include "DS50FragmentGenerator.hh"

#include "cetlib/exception.h"

ds50::DS50FragmentGenerator::DS50FragmentGenerator(): run_number_(-1) {}

void ds50::DS50FragmentGenerator::start (int run) { 
  if (run < 0) throw cet::exception("DS50FragmentGenerator") << "negative run number";
  run_number_ = run; 
  start_ (); 
}

void ds50::DS50FragmentGenerator::stop () { 
  run_number_ = -1;
  stop_ (); 
}

void ds50::DS50FragmentGenerator::pause () { pause_ (); }

void ds50::DS50FragmentGenerator::resume () { resume_ (); } 

std::string ds50::DS50FragmentGenerator::report () { return report_ (); } 

bool ds50::DS50FragmentGenerator::getNext_ (artdaq::FragmentPtrs & output) { 
  if (run_number_ < 0) start (0);
  return getNext__(output); 
}
