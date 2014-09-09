#include "artdaq/ArtModules/BuildInfo_module.hh"

#include "artdaq/Version/GetPackageInfo.h"

namespace artdaq {

  typedef artdaq::BuildInfo<artdaq::PackageInfo> ArtdaqBuildInfo;
  DEFINE_ART_MODULE(ArtdaqBuildInfo)
}
