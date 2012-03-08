#include "artdaq/DAQrate/DS50EventReader.hh"
#include <string>

#include "fhiclcpp/ParameterSet.h"

using fhicl::ParameterSet;

namespace artdaq
{
  DS50EventReader::DS50EventReader(ParameterSet const&)
  {
    throw std::string("Sorry, DS50EventReader does not yet work.");
  }

  DS50EventReader::~DS50EventReader()
  { }

  bool
  DS50EventReader::getNext_(FragmentPtrs&)
  {
    return false;
  }
}
