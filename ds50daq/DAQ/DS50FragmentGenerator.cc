#include "DS50FragmentGenerator.hh"


bool ds50::DS50FragmentGenerator::start () { return start_ (); }
bool ds50::DS50FragmentGenerator::stop () { return stop_ (); }
bool ds50::DS50FragmentGenerator::pause () { return pause_ (); }
bool ds50::DS50FragmentGenerator::resume () { return resume_ (); } std::string ds50::DS50FragmentGenerator::report () { return report_ (); } 
bool ds50::DS50FragmentGenerator::getNext_ (artdaq::FragmentPtrs & output) { return getNext__(output); }
