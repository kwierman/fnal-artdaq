
#include "artdaq/DAQrate/ConcurrentQueue.hh"
#include "artdaq/DAQrate/GlobalQueue.hh"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/ReaderSource.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

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
    vector<string> const        inst_names_;
    daqrate::seconds            waiting_time_;
    bool                        resume_after_timeout_;

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
    inst_names_(ps.get<vector<string>>("instances")),
    waiting_time_(ps.get<double>("waiting_time", std::numeric_limits<double>::infinity())),
    resume_after_timeout_(ps.get<bool>("resume_after_timeout", true))
  {
    for_each(inst_names_.cbegin(), inst_names_.cend(),
             [&](string const & iname) {
               help.reconstitutes<Fragment, art::InEvent>("DS50RawInput", iname);
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

    // Try to get an event from the queue ...
    bool keep_looping = true;
    bool got_event = false;
    while (keep_looping)
      {
        keep_looping = false;
        got_event = queue_.deqTimedWait(p, waiting_time_);
        if (!got_event) {
          mf::LogInfo("InputFailure")
            << "Reading timed out in RawEventQueueReader::readNext()";
          keep_looping = resume_after_timeout_;
        }
      }
    if (!got_event) return false;
    // check for end of data stream
    if (!p) { return false; }
    art::Timestamp runstart;
    // make new runs or subruns if in* are 0 or if the run/subrun
    // have changed
    if (inR == 0 || inR->run() != p->runID()) {
      outR = pm_.makeRunPrincipal(p->runID(),
                                  runstart);
    }
    art::SubRunID subrun_check(p->runID(), p->subrunID());
    if (inSR == 0 || subrun_check != inSR->id()) {
      outSR = pm_.makeSubRunPrincipal(p->runID(),
                                      p->subrunID(),
                                      runstart);
    }
    outE = pm_.makeEventPrincipal(p->runID(),
                                  p->subrunID(),
                                  p->eventID(),
                                  runstart);
    // add all the fragments as products
    if (inst_names_.size() < p->fragments_.size()) {
      throw art::Exception(art::errors::DataCorruption)
        << "more raw data fragments than expected.\n"
        << "expected " << inst_names_.size() << " and got "
        << p->fragments_.size() << "\n"
        << "for event " << p->runID();
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

  typedef art::ReaderSource<RawEventQueueReader> DS50RawInput;
}

DEFINE_ART_INPUT_SOURCE(artdaq::DS50RawInput)
