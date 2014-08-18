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
    art::RunNumber_t current_run_;
  };

  ArtdaqBuildInfo::ArtdaqBuildInfo(fhicl::ParameterSet const &p):
    instance_name_artdaq_(p.get<std::string>("instance_name_artdaq", "buildinfoArtdaq")),
    instance_name_artdaq_core_(p.get<std::string>("instance_name_artdaq_core", "buildinfoArtdaqCore")),
    current_run_(0)
  {
    produces<PackageBuildInfo, art::InRun>(instance_name_artdaq_);
    produces<PackageBuildInfo, art::InRun>(instance_name_artdaq_core_);
  }

  void ArtdaqBuildInfo::beginRun(art::Run &e) { 
    if (e.run () == current_run_) return;
    current_run_ = e.run ();

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
