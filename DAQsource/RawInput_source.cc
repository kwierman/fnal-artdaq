
#include "DAQrate/ConcurrentQueue.hh"
#include "DAQrate/GlobalQueue.hh"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/ReaderSource.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

using std::string;

namespace artdaq {
  struct EventStoreReader {
    art::PrincipalMaker const & pm_;
    RawEventQueue & queue_;
    const vector<string> inst_names_;

    EventStoreReader(fhicl::ParameterSet const & ps,
                     art::ProductRegistryHelper & help,
                     art::PrincipalMaker const & pm);

    void closeCurrentFile();
    void readFile(string const & name, art::FileBlock* & fb);

    bool readNext(art::RunPrincipal* const & inR,
                  art::SubRunPrincipal* const & inSR,
                  art::RunPrincipal* & outR,
                  art::SubRunPrincipal* & outSR,
                  art::EventPrincipal* & outE);
  };

  EventStoreReader::EventStoreReader(fhicl::ParameterSet const & ps,
                                     art::ProductRegistryHelper & help,
                                     art::PrincipalMaker const & pm):
    pm_(pm),
    queue_(getQueue()),
    inst_name_(ps.get<vector<string>>("instances"))
  {
    for_each(inst_names_.cbegin(), inst_names_.cend(),
    [&](string const & iname) {
      help.reconstitutes<Fragment, art::InEvent>("RawInput", iname);
    });
  }

  void EventStoreReader::closeCurrentFile()
  {
  }

  void EventStoreReader::readFile(string const & name,
                                  art::FileBlock* & fb)
  {
    fb = new FileBlock(FileFormatVersion(1, "RawEvent2011"), "nothing");
  }

  bool EventStoreReader::readNext(art::RunPrincipal* const & inR,
                                  art::SubRunPrincipal* const & inSR,
                                  art::RunPrincipal* & outR,
                                  art::SubRunPrincipal* & outSR,
                                  art::EventPrincipal* & outE)
  {
    RawEvent_ptr p;
    queue_.deqWait(p);
    // check for end of data stream
    if (!p) { return false; }
    Timestamp runstart;
    // make new runs or subruns if in* are 0 or if the run/subrun
    // have changed
    if (inR == 0 || inR->run() != p->header_.run_id_) {
      outR = pm_.makeRunPrincipal(p->header_.run_id_,
                                  runstart);
    }
    art::SubRunID subrun_check(p->header_.run_id_, p->header_.subrun_id_);
    if (inSR == 0 || subrun_check != inSR.id()) {
      outSR = pm_.makeSubRunPrincipal(p->header_.run_id_,
                                      p->header_.subrun_id_,
                                      runstart);
    }
    outE = pm_.makeEventPrincipal(p->header_.run_id_,
                                  p->header_.subrun_id_,
                                  p->header_.event_id_,
                                  runstart);
    // add all the fragments as products
    if (inst_name_.size() < p->fragment_list_.size()) {
      throw Exception("RawData")
          << "more raw data fragments than expected.\n"
          << "expected " << inst_name_.size() << " and got "
          << p->fragment_list_.size() << "\n"
          << "for event " << p->header_.run_d_;
    }
    for (auto c = p->fragment_list_.begin(), e = p->fragment_list_.end();
         c != e; ++c) {
      auto_ptr<Fragment> frag(c->release());
      put_product_in_principal(frag, *outE, inst_names_[i]);
    }
    /* example
       std::auto_ptr<std::vector<rawdata::FlatDAQData> > flatcol(new std::vector<rawdata::FlatDAQData>);
       flatcol->push_back(flat_daq_data);
       art::put_product_in_principal(flatcol,*outE,"module","inst");
    */
  }

  typedef art::ReaderSource<EventStoreReader> RawInput;
}

DEFINE_ART_INPUT_SOURCE(artdaq::RawInput)
