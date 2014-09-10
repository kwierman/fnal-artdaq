
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
//#include "artdaq-core/Data/PackageBuildInfo.hh"

#include "artdaq/ArtModules/pkginfo.hh"

#include <iostream>


namespace artdaq {

  template <std::string* instanceName, typename... Pkgs>
  class BuildInfo : public art::EDProducer {
  public:
    explicit BuildInfo(fhicl::ParameterSet const & p);
    virtual ~BuildInfo() {}

    void beginRun(art::Run & r) override;
    void produce(art::Event & e) override;

  private:

    std::unique_ptr< std::vector<pkginfo> > packages_;

    template <typename... Args>
    struct fill_packages;

    template <typename Arg>
    struct fill_packages<Arg> {
      static void doit(const fhicl::ParameterSet &, std::vector<pkginfo>& packages) {

	pkginfo info;

	info.packageName_ = Arg::getPackageName(); 
	info.packageVersion_ = Arg::getPackageVersion();
	info.buildTimestamp_ = Arg::getBuildTimestamp();

	packages.emplace_back( info );

      }
    };

    template <typename Arg, typename... Args>
    struct fill_packages<Arg, Args...> {
      static void doit(const fhicl::ParameterSet & p, std::vector<pkginfo>& packages) {
	
	pkginfo info;
	
	info.packageName_ = Arg::getPackageName(); 
	info.packageVersion_ = Arg::getPackageVersion();
	info.buildTimestamp_ = Arg::getBuildTimestamp();

	packages.emplace_back( info );

	fill_packages<Args...>::doit(p, packages);
      }
    };
    
  };

  template <std::string* instanceName, typename... Pkgs>
  BuildInfo<instanceName, Pkgs...>::BuildInfo(fhicl::ParameterSet const &p):
    packages_( new std::vector<pkginfo>() )
  {

    fill_packages<Pkgs...>::doit(p, *packages_);

    // for (auto package : *packages_) {
    //   std::cout << "Package \"" << package.packageName_ << "\": " << std::endl;
    //   std::cout << "Version: " << package.packageVersion_ << std::endl;
    //   std::cout << "Build time: " << package.buildTimestamp_ << std::endl;
    //   std::cout << std::endl;
    // }

    produces<std::vector<pkginfo>, art::InRun>(*instanceName);

  }

  template <std::string* instanceName, typename... Pkgs>
  void BuildInfo<instanceName, Pkgs...>::beginRun(art::Run &e) { 

    e.put( std::move(packages_), *instanceName );
    
  }

  template <std::string* instanceName, typename... Pkgs>
  void BuildInfo<instanceName, Pkgs...>::produce(art::Event &)
  {
    // nothing to be done for individual events
  }


}
