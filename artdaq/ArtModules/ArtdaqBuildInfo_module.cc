#include "artdaq/ArtModules/BuildInfo_module.hh"

#include "artdaq/BuildInfo/GetPackageInfo.h"

#include <string>

namespace artdaq {

  static std::string instanceName = "ArtdaqBuildInfo";
  typedef artdaq::BuildInfo< &instanceName, artdaq::PackageInfo> ArtdaqBuildInfo;

  DEFINE_ART_MODULE(ArtdaqBuildInfo)
}
