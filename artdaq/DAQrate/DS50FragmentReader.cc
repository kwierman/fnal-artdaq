#include "artdaq/DAQrate/DS50FragmentReader.hh"
#include <string>

#include "fhiclcpp/ParameterSet.h"

using fhicl::ParameterSet;

namespace artdaq
{
  DS50FragmentReader::DS50FragmentReader(ParameterSet const&)
  {
    throw std::string("Sorry, DS50FragmentReader does not yet work.");
  }

  DS50FragmentReader::~DS50FragmentReader()
  { }

  bool
  DS50FragmentReader::getNext_(FragmentPtrs&)
  {
    return false;
  }
}
