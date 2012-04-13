#include "artdaq/DAQdata/FragmentGenerator.hh"

namespace artdaq {
  FragmentGenerator::~FragmentGenerator()
  { }

  bool
  FragmentGenerator::getNext(FragmentPtrs & output)
  {
    return getNext_(output);
  }
}
