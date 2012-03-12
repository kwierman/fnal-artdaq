
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
  namespace detail {
    struct RawEventQueueReader {
      RawEventQueueReader(RawEventQueueReader const&) = delete;
      RawEventQueueReader& operator=(RawEventQueueReader const&) = delete;

      art::PrincipalMaker const & pmaker;
      RawEventQueue &             incoming_events;
      vector<string> const        product_instance_names;
      daqrate::seconds            waiting_time;
      bool                        resume_after_timeout;

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
  }

  detail::RawEventQueueReader::RawEventQueueReader(fhicl::ParameterSet const & ps,
                                                   art::ProductRegistryHelper & help,
                                                   art::PrincipalMaker const & pm):
    pmaker(pm),
    incoming_events(getGlobalQueue()),
    product_instance_names(ps.get<vector<string>>("instances")),
    waiting_time(ps.get<double>("waiting_time", std::numeric_limits<double>::infinity())),
    resume_after_timeout(ps.get<bool>("resume_after_timeout", true))
  {
    for (auto const& iname : product_instance_names)
      help.reconstitutes<Fragment, art::InEvent>("DS50RawInput", iname);
//     for_each(product_instance_names.cbegin(), product_instance_names.cend(),
//              [&](string const & iname) {
//                help.reconstitutes<Fragment, art::InEvent>("DS50RawInput", iname);
//              });
  }

  inline
  void detail::RawEventQueueReader::closeCurrentFile()
  {
  }

  void detail::RawEventQueueReader::readFile(string const & /* name */,
                                     art::FileBlock* & fb)
  {
    fb = new art::FileBlock(art::FileFormatVersion(1, "RawEvent2011"), "nothing");
  }

  bool detailRawEventQueueReader::readNext(art::RunPrincipal* const & inR,
                                           art::SubRunPrincipal* const & inSR,
                                           art::RunPrincipal* & outR,
                                           art::SubRunPrincipal* & outSR,
                                           art::EventPrincipal* & outE)
  {
    RawEvent_ptr p;

    // Try to get an event from the queue. We'll continuously loop, either until:
    //   1) we have read a RawEvent off the queue, or
    //   2) we have timed out, AND we are told the when we timeout we
    //      should stop.
    // In any case, if we time out, we emit an informational message.
    bool keep_looping = true;
    bool got_event = false;
    while (keep_looping)
      {
        keep_looping = false;
        got_event = incoming_events.deqTimedWait(p, waiting_time);
        if (!got_event) {
          mf::LogInfo("InputFailure")
            << "Reading timed out in RawEventQueueReader::readNext()";
          keep_looping = resume_after_timeout;
        }
      }

    // We return false, indicating we're done reading, if:

    //   1) we did not obtain an event, because we timed out and were
    //      configured NOT to keep trying after a timeout, or
    //   2) the event we read was the end-of-data marker: a null
    //      pointer

    if (!got_event || !p) return false;

    art::Timestamp runstart;
    // make new runs or subruns if in* are 0 or if the run/subrun
    // have changed
    if (inR == 0 || inR->run() != p->runID()) {
      outR = pmaker.makeRunPrincipal(p->runID(),
                                     runstart);
    }
    art::SubRunID subrun_check(p->runID(), p->subrunID());
    if (inSR == 0 || subrun_check != inSR->id()) {
      outSR = pmaker.makeSubRunPrincipal(p->runID(),
                                      p->subrunID(),
                                      runstart);
    }
    outE = pmaker.makeEventPrincipal(p->runID(),
                                  p->subrunID(),
                                  p->eventID(),
                                  runstart);

    // add all the fragments as products
    if (product_instance_names.size() < p->fragments_.size()) {
      throw art::Exception(art::errors::DataCorruption)
        << "more raw data fragments than expected.\n"
        << "expected " << product_instance_names.size() << " and got "
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
      put_product_in_principal(frag, *outE, product_instance_names[i]);
    }

    return true;
  }

  typedef art::ReaderSource<RawEventQueueReader> DS50RawInput;
}

DEFINE_ART_INPUT_SOURCE(artdaq::DS50RawInput)
