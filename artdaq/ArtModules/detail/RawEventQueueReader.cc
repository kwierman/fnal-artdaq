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

  // 18-Feb-2013, KAB: added support for mapping fragments into
  // user-specified types.  Here is a sample FHICL parameter
  // definition this functionality:
  // fragment_type_map: [[17, \"V172X\"], [18, \"V1495\"]]
  //
  // Fetch the user-specified fragment mapping
  std::vector<std::vector<std::string>> fragmentMapParams;
  if (ps.get_if_present<std::vector<std::vector<std::string>>>
      ("fragment_type_map", fragmentMapParams)) {
    for (int idx = 0; idx < (int) fragmentMapParams.size(); ++idx) {
      std::vector<std::string> the_pair = fragmentMapParams[idx];
      std::string type_id_string = the_pair[0];
      std::string type_name = the_pair[1];
      try {
        // why doesn't this work??
        //Fragment::type_t type_id =
        //  boost::lexical_cast<Fragment::type_t>(type_id_string);
        unsigned short type_id =
          boost::lexical_cast<unsigned short>(type_id_string);
        Fragment::type_t type_id2 = static_cast<Fragment::type_t>(type_id);
        fragment_type_map_[type_id2] = type_name;
        help.reconstitutes<Fragments, art::InEvent>(pretend_module_name, type_name);
      }
      catch (...) {
        mf::LogError("InvalidParameterValue")
          << "Invalid value for fragment type id: \""
          << type_id_string << "\"";
      }
    }
  }
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
  // get the list of fragment types that exist in the event
  std::vector<Fragment::type_t> type_list;
  popped_event->fragmentTypes(type_list);
  // insert the Fragments of each type into the EventPrincipal
  std::map<Fragment::type_t, std::string>::const_iterator iter_end =
    fragment_type_map_.end();
  for (size_t idx = 0; idx < type_list.size(); ++idx) {
    std::map<Fragment::type_t, std::string>::const_iterator iter =
      fragment_type_map_.find(type_list[idx]);
    if (iter != iter_end) {
      put_product_in_principal(popped_event->releaseProduct(type_list[idx]),
                               *outE,
                               pretend_module_name,
                               iter->second);
    }
    else {
      put_product_in_principal(popped_event->releaseProduct(type_list[idx]),
                               *outE,
                               pretend_module_name);
    }
  }

  return true;
}
