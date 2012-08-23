#include "artdaq/ArtModules/detail/RawEventQueueReader.hh"

#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Utilities/Exception.h"
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

using std::string;

artdaq::detail::RawEventQueueReader::RawEventQueueReader(fhicl::ParameterSet const & ps,
    art::ProductRegistryHelper & help,
    art::PrincipalMaker const & pm):
  pmaker(pm),
  incoming_events(getGlobalQueue()),
  waiting_time(ps.get<double>("waiting_time", std::numeric_limits<double>::infinity())),
  resume_after_timeout(ps.get<bool>("resume_after_timeout", true)),
  pretend_module_name("daq")
{
  help.reconstitutes<Fragments, art::InEvent>(pretend_module_name);
}

void artdaq::detail::RawEventQueueReader::closeCurrentFile()
{
}

void artdaq::detail::RawEventQueueReader::readFile(string const &,
    art::FileBlock *& fb)
{
  fb = new art::FileBlock(art::FileFormatVersion(1, "RawEvent2011"), "nothing");
}

bool artdaq::detail::RawEventQueueReader::readNext(art::RunPrincipal * const & inR,
    art::SubRunPrincipal * const & inSR,
    art::RunPrincipal *& outR,
    art::SubRunPrincipal *& outSR,
    art::EventPrincipal *& outE)
{
  // Establish default 'results'
  outR = 0;
  outSR = 0;
  outE = 0;
  RawEvent_ptr popped_event;
  // Try to get an event from the queue. We'll continuously loop, either until:
  //   1) we have read a RawEvent off the queue, or
  //   2) we have timed out, AND we are told the when we timeout we
  //      should stop.
  // In any case, if we time out, we emit an informational message.
  bool keep_looping = true;
  bool got_event = false;
  while (keep_looping) {
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
  if (!got_event || !popped_event) { return false; }
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
