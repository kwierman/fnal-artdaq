////////////////////////////////////////////////////////////////////////
// Class:       PrintVersionInfo
// Module Type: analyzer
// File:        PrintVersionInfo_module.cc
//
// Generated at Fri Aug 15 21:05:07 2014 by lbnedaq using artmod
// from cetpkgsupport v1_05_02.
////////////////////////////////////////////////////////////////////////


// JCF, 8/19/14                                                                                                          

// As the following function:                                                                                           

// void analyze(art::Event const & e)                                                                                    

// in PrintVersionInfo's base class, art::EDAnalyzer, needs to be
// overridden, but this module only analyzes run-by-run info rather
// than event-by-event info, there's no point in adding an
// implementation body to the function. Thus, for this module we
// disable the error condition usually called by an unused function
// parameter (here, the reference-to-art::Event)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"


#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include "artdaq-core/Data/PackageBuildInfo.hh"

#include <iostream>

namespace artdaq {
  class PrintVersionInfo;
}

class artdaq::PrintVersionInfo : public art::EDAnalyzer {
public:
  explicit PrintVersionInfo(fhicl::ParameterSet const & p);
  virtual ~PrintVersionInfo();

  void analyze(art::Event const & e) override { 
    if (e.run() == 99999999) {
      std::cout << "JCF, 8/16/16 -- the only reason the analyze() function in PrintVersionInfo has a body is because the compiler complains if the art::Event argument goes unused" << std::endl;
    }
  }

  virtual void beginRun(art::Run const& r);


private:

  std::string artdaq_module_label_;
  std::string artdaq_instance_label_;

  std::string artdaq_core_module_label_;
  std::string artdaq_core_instance_label_;

};


artdaq::PrintVersionInfo::PrintVersionInfo(fhicl::ParameterSet const & pset)
  :
  EDAnalyzer(pset),
  artdaq_module_label_(pset.get<std::string>("artdaq_module_label")),
  artdaq_instance_label_(pset.get<std::string>("artdaq_instance_label")),
  artdaq_core_module_label_(pset.get<std::string>("artdaq_core_module_label")),
  artdaq_core_instance_label_(pset.get<std::string>("artdaq_core_instance_label"))
{}

artdaq::PrintVersionInfo::~PrintVersionInfo()
{
  // Clean up dynamic memory and other resources here.
}


void artdaq::PrintVersionInfo::beginRun(art::Run const& run)
{

  art::Handle<artdaq::PackageBuildInfo> raw;


  run.getByLabel(artdaq_core_module_label_, artdaq_core_instance_label_, raw);

  if (raw.isValid()) {
    std::cout << std::endl;
    std::cout << "artdaq-core package version: " << raw->getPackageVersion() << std::endl;
    std::cout << "artdaq-core package build time: " << raw->getBuildTimestamp() << std::endl;
    std::cout << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << "Warning in artdaq::PrintVersionInfo module: Run " << run.run() << " appears to have no info on artdaq-core build time / version number" << std::endl;
    std::cerr << std::endl;
  }


  run.getByLabel(artdaq_module_label_, artdaq_instance_label_, raw);

  if (raw.isValid()) {
    std::cout << std::endl;
    std::cout << "artdaq package version: " << raw->getPackageVersion() << std::endl;
    std::cout << "artdaq package build time: " << raw->getBuildTimestamp() << std::endl;
    std::cout << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << "Warning in artdaq::PrintVersionInfo module: Run " << run.run() << " appears to have no info on artdaq build time / version number" << std::endl;
    std::cerr << std::endl;
  }

}

// Restore "unused-parameter" as a compiler error
#pragma GCC diagnostic pop 

DEFINE_ART_MODULE(artdaq::PrintVersionInfo)
