#ifndef artdaq_BuildInfo_GetPackageInfo_h
#define artdaq_BuildInfo_GetPackageInfo_h

#include "artdaq-core/Data/PackageBuildInfo.hh"

#include <string>

namespace artdaq {

  struct PackageInfo {

    static artdaq::PackageBuildInfo getPackageBuildInfo();
  };

}

#endif /* artdaq_BuildInfo_GetPackageInfo_h */
