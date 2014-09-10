////////////////////////////////////////////////////////////////////////
// Class:       PrintVersionInfo
// Module Type: analyzer
// File:        PrintVersionInfo_module.cc
//
// Generated at Fri Aug 15 21:05:07 2014 by lbnedaq using artmod
// from cetpkgsupport v1_05_02.
////////////////////////////////////////////////////////////////////////



#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

//#include "artdaq-core/Data/PackageBuildInfo.hh"
#include "artdaq/ArtModules/pkginfo.hh"

#include <iostream>

namespace artdaq {
  class PrintVersionInfo;
}

class artdaq::PrintVersionInfo : public art::EDAnalyzer {
public:
  explicit PrintVersionInfo(fhicl::ParameterSet const & p);
  virtual ~PrintVersionInfo();

  void analyze(art::Event const & ) override { 

  }

  virtual void beginRun(art::Run const& r);

private:

  std::string buildinfo_module_label_;
  std::string buildinfo_instance_label_;

};


artdaq::PrintVersionInfo::PrintVersionInfo(fhicl::ParameterSet const & pset)
  :
  EDAnalyzer(pset),
  buildinfo_module_label_(pset.get<std::string>("buildinfo_module_label")),
  buildinfo_instance_label_(pset.get<std::string>("buildinfo_instance_label"))
{}

artdaq::PrintVersionInfo::~PrintVersionInfo()
{
  // Clean up dynamic memory and other resources here.
}


void artdaq::PrintVersionInfo::beginRun(art::Run const& run)
{

  art::Handle<std::vector<artdaq::pkginfo> > raw;

  run.getByLabel(buildinfo_module_label_, buildinfo_instance_label_, raw);

  if (raw.isValid()) {

    for (auto pkg : *raw ) {
      std::cout << std::endl;
      std::cout << "Package " << pkg.packageName_ << ": " << std::endl;
      std::cout << "Version: " << pkg.packageVersion_ << std::endl;
      std::cout << "Timestamp: " << pkg.buildTimestamp_ << std::endl;
      std::cout << std::endl;
    }
    

  } else {

    std::cerr << "\n" << std::endl;
    std::cerr << "Warning in artdaq::PrintVersionInfo module: Run " << run.run() << 
      " appears not to have found product instance \"" << buildinfo_instance_label_ << 
      "\" of module \"" << buildinfo_module_label_ << "\"" << std::endl;
    std::cerr << "\n" << std::endl;

  }

}

DEFINE_ART_MODULE(artdaq::PrintVersionInfo)
