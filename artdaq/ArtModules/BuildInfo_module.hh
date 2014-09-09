
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "artdaq-core/Data/PackageBuildInfo.hh"
#include "artdaq/Version/GetReleaseVersion.h"
#include "artdaq-core/Version/GetReleaseVersion.h"

#include <iostream>


namespace artdaq {

  template <typename... Pkgs>
  class BuildInfo : public art::EDProducer {
  public:
    explicit BuildInfo(fhicl::ParameterSet const & p);
    virtual ~BuildInfo() {}

    void beginRun(art::Run & r) override;
    void produce(art::Event & e) override;

  private:

    struct pkginfo {

      std::string instanceName_;
      std::string packageVersion_;
      std::string buildTimestamp_;
    };

    std::vector<pkginfo> packages;

    template <typename... Args>
    struct fill_packages;

    template <typename Arg>
    struct fill_packages<Arg> {
      static void doit(const fhicl::ParameterSet &p, std::vector<pkginfo>& packages) {

	pkginfo info;
	
	std::string instanceLabel = static_cast<std::string>("instance_name_") + Arg::getPackageName();
	std::string defaultInstanceName = Arg::getPackageName() + static_cast<std::string>("BuildInfo");

	info.instanceName_ = p.get<std::string>(instanceLabel, defaultInstanceName); 

	info.packageVersion_ = Arg::getPackageVersion();
	info.buildTimestamp_ = Arg::getBuildTimestamp();

	packages.emplace_back( info );

      }
    };

    template <typename Arg, typename... Args>
    struct fill_packages<Arg, Args...> {
      static void doit(const fhicl::ParameterSet &p, std::vector<pkginfo>& packages) {
	
	pkginfo info;
	
	std::string instanceLabel = static_cast<std::string>("instance_name_") + Arg::getPackageName();
	std::string defaultInstanceName = Arg::getPackageName() + static_cast<std::string>("BuildInfo");	

	info.instanceName_ = p.get<std::string>(instanceLabel, defaultInstanceName); 

	info.packageVersion_ = Arg::getPackageVersion();
	info.buildTimestamp_ = Arg::getBuildTimestamp();

	packages.emplace_back( info );

	fill_packages<Args...>::doit(p, packages);
      }
    };
    
  };

  template <typename... Pkgs>
  BuildInfo<Pkgs...>::BuildInfo(fhicl::ParameterSet const &p)
  {
    
    fill_packages<Pkgs...>::doit(p, packages);

    // JCF, 9/7/14

    // This is a printout to be used only during development; I just
    // want to make sure the build time and package version got into
    // my array of structs

    for (auto package : packages) {
      std::cout << "Instance \"" << package.instanceName_ << "\": " << std::endl;
      std::cout << "Version: " << package.packageVersion_ << std::endl;
      std::cout << "Build time: " << package.buildTimestamp_ << std::endl;
      std::cout << std::endl;

      produces<PackageBuildInfo, art::InRun>(package.instanceName_);
    }

  }

  template <typename... Pkgs>
  void BuildInfo<Pkgs...>::beginRun(art::Run &e) { 

    for (auto package : packages ) {
      std::unique_ptr<PackageBuildInfo> build_info(new PackageBuildInfo());
      build_info->setPackageVersion(package.packageVersion_);
      build_info->setBuildTimestamp(package.buildTimestamp_);

      e.put(std::move(build_info), package.instanceName_ );
    }
  }

  template <typename... Pkgs>
  void BuildInfo<Pkgs...>::produce(art::Event &)
  {
    // nothing to be done for individual events
  }


}
