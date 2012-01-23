#include "DS50EventReader.hh"
#include <string>

using fhicl::ParameterSet;

namespace artdaq
{
  DS50EventReader::DS50EventReader(ParameterSet const& ps) :
    do_random_(ps.get<bool>("do_random"))
  {
    if (!do_random_) 
      throw std::string("Only random generation supported.");
  }
}
