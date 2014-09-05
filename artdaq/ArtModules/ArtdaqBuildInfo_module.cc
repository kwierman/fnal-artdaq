#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq-core/Data/PackageBuildInfo.hh"
#include "artdaq/Version/GetReleaseVersion.h"
#include "artdaq-core/Version/GetReleaseVersion.h"

namespace artdaq {

  class ArtdaqBuildInfo : public art::EDProducer {
  public:
    explicit ArtdaqBuildInfo(fhicl::ParameterSet const & p);
    virtual ~ArtdaqBuildInfo() {}

    void beginRun(art::Run & r) override;
    void produce(art::Event & e) override;

  private:
    std::string const instance_name_artdaq_;
    std::string const instance_name_artdaq_core_;
  };

  ArtdaqBuildInfo::ArtdaqBuildInfo(fhicl::ParameterSet const &p):
    instance_name_artdaq_(p.get<std::string>("instance_name_artdaq", "buildinfoArtdaq")),
    instance_name_artdaq_core_(p.get<std::string>("instance_name_artdaq_core", "buildinfoArtdaqCore"))
  {
    produces<PackageBuildInfo, art::InRun>(instance_name_artdaq_);
    produces<PackageBuildInfo, art::InRun>(instance_name_artdaq_core_);
  }

  void ArtdaqBuildInfo::beginRun(art::Run &e) { 
    // 19-Aug-2014, KAB: Removed the check on whether the run number
    // has changed.  We want the run data products to be added to each
    // file, and since the beginRun() method is called for each file,
    // the code in this method should take care of that.  If/when the
    // callbacks within art are changed so that beginRun() is only
    // called when a new run is encountered (not a new file), then we
    // may need to move this code to the appropriate (new?) callback.

    std::string s1, s2;

    // create a PackageBuildInfo object for artdaq and add it to the Run
    std::unique_ptr<PackageBuildInfo> build_info_artdaq(new PackageBuildInfo());
    s1 = artdaq::getReleaseVersion();
    build_info_artdaq->setPackageVersion(s1);
    s2 = artdaq::getBuildDateTime();
    build_info_artdaq->setBuildTimestamp(s2);

    e.put(std::move(build_info_artdaq),instance_name_artdaq_);

    // And do the same for the artdaq-core package on which it depends

    std::unique_ptr<PackageBuildInfo> build_info_artdaq_core(new PackageBuildInfo());
    s1 = artdaqcore::getReleaseVersion();
    build_info_artdaq_core->setPackageVersion(s1);
    s2 = artdaqcore::getBuildDateTime();
    build_info_artdaq_core->setBuildTimestamp(s2);

    e.put(std::move(build_info_artdaq_core),instance_name_artdaq_core_);

  }

  void ArtdaqBuildInfo::produce(art::Event &)
  {
    // nothing to be done for individual events
  }

  DEFINE_ART_MODULE(ArtdaqBuildInfo)
}
