#include "art/Framework/IO/Sources/ReaderSource.h"
#include "artdaq/DAQsource/detail/RawEventQueueReader.hh"
#include "art/Framework/Core/InputSourceMacros.h"

namespace artdaq {

  namespace detail {
    struct RawEventQueueReader {
      RawEventQueueReader(RawEventQueueReader const&) = delete;
      RawEventQueueReader& operator=(RawEventQueueReader const&) = delete;

      art::PrincipalMaker const & pmaker;
      RawEventQueue &             incoming_events;
      daqrate::seconds            waiting_time;
      bool                        resume_after_timeout;
      std::string                 pretend_module_name;

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
    waiting_time(ps.get<double>("waiting_time", std::numeric_limits<double>::infinity())),
    resume_after_timeout(ps.get<bool>("resume_after_timeout", true)),
    pretend_module_name("daq")
  {
    help.reconstitutes<std::vector<Fragment>, art::InEvent>(pretend_module_name);
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

  bool detail::RawEventQueueReader::readNext(art::RunPrincipal* const & inR,
                                             art::SubRunPrincipal* const & inSR,
                                             art::RunPrincipal* & outR,
                                             art::SubRunPrincipal* & outSR,
                                             art::EventPrincipal* & outE)
  {
    RawEvent_ptr popped_event;

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
        got_event = incoming_events.deqTimedWait(popped_event, waiting_time);
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

    if (!got_event || !popped_event) return false;

    art::Timestamp runstart;
    // make new runs or subruns if in* are 0 or if the run/subrun
    // have changed
    if (inR == 0 || inR->run() != popped_event->runID()) {
      outR = pmaker.makeRunPrincipal(popped_event->runID(),
                                     runstart);
    }
    art::SubRunID subrun_check(popped_event->runID(), popped_event->subrunID());
    if (inSR == 0 || subrun_check != inSR->id()) {
      outSR = pmaker.makeSubRunPrincipal(popped_event->runID(),
                                         popped_event->subrunID(),
                                         runstart);
    }
    outE = pmaker.makeEventPrincipal(popped_event->runID(),
                                     popped_event->subrunID(),
                                     popped_event->sequenceID(),
                                     runstart);

    // Finally, grab the Fragments out of the RawEvent, and insert
    // them into the EventPrincipal.
    put_product_in_principal(popped_event->releaseProduct(),
                             *outE,
                             pretend_module_name);
    return true;
  }

  typedef art::ReaderSource<detail::RawEventQueueReader> RawInput;
}

DEFINE_ART_INPUT_SOURCE(artdaq::RawInput)
