
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Provenance/BranchType.h"

#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/Fragments.hh"

#include <string>

namespace artdaq
{

  class FragmentSniffer : public art::EDAnalyzer
  {
  public:
    explicit FragmentSniffer( fhicl::ParameterSet const& p );
    virtual ~FragmentSniffer() { };
    
    virtual void analyze(art::Event const& e);
    virtual void endSubRun(art::SubRun const& sr);
    virtual void endRun(art::Run const& r);
    virtual void endJob();

  private:
    std::string raw_label_;
    std::size_t num_frags_per_event_;
    std::size_t num_events_expected_;
    std::size_t num_events_processed_;
  };

  FragmentSniffer::FragmentSniffer( fhicl::ParameterSet const& p ) :
    raw_label_( p.get<std::string>("raw_label") ),
    num_frags_per_event_(p.get<size_t>("num_frags_per_event")),
    num_events_expected_(p.get<size_t>("num_events_expected")),
    num_events_processed_()
  {
    std::cerr << "FS::FragmentSniffer: module created" << std::endl;
  }

  void FragmentSniffer::analyze(art::Event const& e)
  {
    std::cerr << "FS::analyze()\n";

    art::Handle<Fragments> handle;
    e.getByLabel(raw_label_, handle);
    assert(handle->empty() || "getByLabel returned empty handle");
    assert(handle->size() == num_frags_per_event_);
    ++num_events_processed_;

    std::cerr << "FS: " << num_events_processed_ << std::endl;;
    LOG_DEBUG("FS") << num_events_processed_ << std::endl;;
  }

  void FragmentSniffer::endSubRun(art::SubRun const&) { }
  void FragmentSniffer::endRun(art::Run const&) { }
  void FragmentSniffer::endJob()
  {
    std::cerr << "FS::endJob()\n";
    std::cerr << "FS: events processed: " << num_events_processed_ << std::endl;;
    std::cerr << "FS: events expected:  " << num_events_expected_ << std::endl;;
    assert(num_events_processed_ == num_events_expected_);
  }

  DEFINE_ART_MODULE(FragmentSniffer)
}

