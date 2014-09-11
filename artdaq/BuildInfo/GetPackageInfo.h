#ifndef artdaq_BuildInfo_GetPackageInfo_h
#define artdaq_BuildInfo_GetPackageInfo_h

#include <string>

namespace artdaq {

  struct PackageInfo {

    static std::string getPackageName();
    static std::string getPackageVersion(); 
    static std::string getBuildTimestamp();
  };

}

#endif /* artdaq_BuildInfo_GetPackageInfo_h */
