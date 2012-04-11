#include "artdaq/DAQrate/FragmentGenerator.hh"

namespace artdaq {
  FragmentGenerator::~FragmentGenerator()
  { }

  bool
  FragmentGenerator::getNext(FragmentPtrs & output)
  {
    return getNext_(output);
  }
}
