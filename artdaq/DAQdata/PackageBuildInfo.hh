#ifndef artdaq_DAQdata_PackageBuildInfo_hh
#define artdaq_DAQdata_PackageBuildInfo_hh

#include <string>

namespace artdaq {
  class PackageBuildInfo;
}

class artdaq::PackageBuildInfo {
public:
  explicit PackageBuildInfo();

  std::string getPackageVersion() const { return packageVersion_; }
  std::string getBuildTimestamp() const { return buildTimestamp_; }
  void setPackageVersion(std::string str) { packageVersion_ = str; }
  void setBuildTimestamp(std::string str) { buildTimestamp_ = str; }

private:

  std::string packageVersion_;
  std::string buildTimestamp_;
};

#endif /* artdaq_DAQdata_PackageBuildInfo_hh */
