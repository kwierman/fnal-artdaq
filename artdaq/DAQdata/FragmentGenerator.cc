#include "artdaq/DAQdata/FragmentGenerator.hh"

namespace artdaq {
  bool
  FragmentGenerator::getNext(FragmentPtrs & output)
  {
    return getNext_(output);
  }
}
