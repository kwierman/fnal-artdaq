#include "artdaq/DAQrate/DS50EventGenerator.hh"

namespace artdaq
{
  DS50EventGenerator::~DS50EventGenerator()
  { }

  bool
  DS50EventGenerator::getNext(FragmentPtrs& output)
  {
    return getNext_(output);
  }
}
