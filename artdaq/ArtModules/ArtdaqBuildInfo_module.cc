#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq-core/Data/PackageBuildInfo.hh"
#include "artdaq/Version/GetReleaseVersion.h"

namespace artdaq {

  class ArtdaqBuildInfo : public art::EDProducer {
  public:
    explicit ArtdaqBuildInfo(fhicl::ParameterSet const & p);
    virtual ~ArtdaqBuildInfo() {}

    void beginRun(art::Run & r) override;
    void produce(art::Event & e) override;

  private:
    std::string const inst_name_;
    art::RunNumber_t current_run_;
  };

  ArtdaqBuildInfo::ArtdaqBuildInfo(fhicl::ParameterSet const &p):
    inst_name_(p.get<std::string>("instance_name", "buildinfo")),
    current_run_(0)
  {
    produces<PackageBuildInfo, art::InRun>(inst_name_);
  }

  void ArtdaqBuildInfo::beginRun(art::Run &e) { 
    if (e.run () == current_run_) return;
    current_run_ = e.run ();

    // create a PackageBuildInfo object and add it to the Run
    std::unique_ptr<PackageBuildInfo> build_info(new PackageBuildInfo());
    std::string s1(artdaq::getReleaseVersion());
    build_info->setPackageVersion(s1);
    std::string s2(artdaq::getBuildDateTime());
    build_info->setBuildTimestamp(s2);

    e.put(std::move(build_info),inst_name_);
  }

  void ArtdaqBuildInfo::produce(art::Event &)
  {
    // nothing to be done for individual events
  }

  DEFINE_ART_MODULE(ArtdaqBuildInfo)
}
