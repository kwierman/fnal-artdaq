#include "DS50EventReader.hh"
#include <string>

#include "fhiclcpp/ParameterSet.h"

using fhicl::ParameterSet;

const std::size_t NUM_BOARDS = 5; // Should this be a configuration parameter?

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
