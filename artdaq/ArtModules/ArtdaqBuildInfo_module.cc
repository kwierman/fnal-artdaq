#include "artdaq/ArtModules/BuildInfo_module.hh"

#include "artdaq/Version/GetPackageInfo.h"

#include <string>

namespace artdaq {

  //static const char* instanceName = "ArtdaqBuildInfo";

  //  static std::string* instanceName = new std::string( "ArtdaqBuildInfo" );
  static std::string instanceName = "ArtdaqBuildInfo";
  typedef artdaq::BuildInfo< &instanceName, artdaq::PackageInfo> ArtdaqBuildInfo;
  DEFINE_ART_MODULE(ArtdaqBuildInfo)
}
