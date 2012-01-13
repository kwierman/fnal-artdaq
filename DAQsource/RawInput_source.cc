
#include "DAQrate/ConcurrentQueue.hh"
#include "DAQrate/GlobalQueue.hh"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/ReaderSource.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <vector>
#include <string>


using std::string;
using std::vector;
using std::auto_ptr;

namespace artdaq {
  struct RawEventQueueReader {
    art::PrincipalMaker const & pm_;
    RawEventQueue &             queue_;
    const vector<string>        inst_names_;

    RawEventQueueReader(fhicl::ParameterSet const & ps,
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

  RawEventQueueReader::RawEventQueueReader(fhicl::ParameterSet const & ps,
                                           art::ProductRegistryHelper & help,
                                           art::PrincipalMaker const & pm):
    pm_(pm),
    queue_(getGlobalQueue()),
    inst_names_(ps.get<vector<string>>("instances"))
  {
    for_each(inst_names_.cbegin(), inst_names_.cend(),
             [&](string const & iname) {
               help.reconstitutes<Fragment, art::InEvent>("RawInput", iname);
             });
  }

  void RawEventQueueReader::closeCurrentFile()
  {
  }

  void RawEventQueueReader::readFile(string const & /* name */,
                                     art::FileBlock* & fb)
  {
    fb = new art::FileBlock(art::FileFormatVersion(1, "RawEvent2011"), "nothing");
  }

  bool RawEventQueueReader::readNext(art::RunPrincipal* const & inR,
                                     art::SubRunPrincipal* const & inSR,
                                     art::RunPrincipal* & outR,
                                     art::SubRunPrincipal* & outSR,
                                     art::EventPrincipal* & outE)
  {
    RawEvent_ptr p;
    queue_.deqWait(p);
    // check for end of data stream
    if (!p) { return false; }
    art::Timestamp runstart;
    // make new runs or subruns if in* are 0 or if the run/subrun
    // have changed
    if (inR == 0 || inR->run() != p->header_.run_id_) {
      outR = pm_.makeRunPrincipal(p->header_.run_id_,
                                  runstart);
    }
    art::SubRunID subrun_check(p->header_.run_id_, p->header_.subrun_id_);
    if (inSR == 0 || subrun_check != inSR->id()) {
      outSR = pm_.makeSubRunPrincipal(p->header_.run_id_,
                                      p->header_.subrun_id_,
                                      runstart);
    }
    outE = pm_.makeEventPrincipal(p->header_.run_id_,
                                  p->header_.subrun_id_,
                                  p->header_.event_id_,
                                  runstart);
    // add all the fragments as products
    if (inst_names_.size() < p->fragments_.size()) {
      throw art::Exception(art::errors::DataCorruption)
        << "more raw data fragments than expected.\n"
        << "expected " << inst_names_.size() << " and got "
        << p->fragments_.size() << "\n"
        << "for event " << p->header_.run_id_;
    }
    for (size_t i = 0, sz = p->fragments_.size();
         i < sz; ++i) {
      // PROBLEM! std::shared_ptr instances cannot release their
      // controlled objects, so we have to copy them into the Event
      // (by copying to the std::auto_ptr). Look into using
      // std::unique_ptr, and move semantics to move into and out of
      // the shared queue.
      auto_ptr<Fragment> frag(new Fragment(*p->fragments_[i]));
      put_product_in_principal(frag, *outE, inst_names_[i]);
    }

    return true;
  }

  typedef art::ReaderSource<RawEventQueueReader> RawInput;
}

DEFINE_ART_INPUT_SOURCE(artdaq::RawInput)
