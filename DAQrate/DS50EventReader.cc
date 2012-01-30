#include "DS50EventReader.hh"
#include <string>

#include "fhiclcpp/ParameterSet.h"

using fhicl::ParameterSet;

namespace artdaq
{
  DS50EventReader::DS50EventReader(ParameterSet const& ps) :
    do_random_(ps.get<bool>("do_random"))
  {
    if (!do_random_) 
      throw std::string("Only random generation supported.");
  }

  bool
  DS50EventReader::getNext(FragmentPtrs& output)
  {
    return (do_random_) ? getNext_random_(output) : getNext_read_(output);
  }

  bool
  DS50EventReader::getNext_random_(FragmentPtrs&)
  {
    return false;
  }

  bool
  DS50EventReader::getNext_read_(FragmentPtrs&)
  {
      throw std::string("Only random generation supported.");
  }

}
