#ifndef artdaq_Version_GetReleaseVersion_h
#define artdaq_Version_GetReleaseVersion_h

#include <string>

namespace artdaq {
  std::string const & getReleaseVersion();
  std::string const & getBuildDateTime();
}

#endif /* artdaq_Version_GetReleaseVersion_h */

// Local Variables:
// mode: c++
// End:
