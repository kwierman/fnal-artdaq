#ifndef artdaq_BuildInfo_GetReleaseVersion_h
#define artdaq_BuildInfo_GetReleaseVersion_h

#include <string>

namespace artdaq {
  std::string const & getReleaseVersion();
  std::string const & getBuildDateTime();
}

#endif /* artdaq_BuildInfo_GetReleaseVersion_h */

// Local Variables:
// mode: c++
// End:
