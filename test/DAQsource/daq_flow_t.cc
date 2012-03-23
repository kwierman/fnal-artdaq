#include "artdaq/DAQrate/EventStore.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQrate/GenericFragmentSimulator.hh"
#include "art/Framework/Art/artapp.h"
#include "fhiclcpp/ParameterSet.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

using artdaq::EventStore;
using artdaq::FragmentPtrs;
using artdaq::GenericFragmentSimulator;
using fhicl::ParameterSet;
using std::size_t;


int main(int argc, char* argv[]) {
  int rc = -1;
  try {

    size_t const NUM_FRAGS_PER_EVENT = 5;
    EventStore::run_id_t const RUN_ID = 2112;
    int const STORE_ID = 1;

    // We may want to add ParameterSet parsing to this code, but right
    // now this will do...
    ParameterSet sim_config;
    sim_config.put("events_to_generate", 100);
    sim_config.put("fragments_per_event", NUM_FRAGS_PER_EVENT);
    sim_config.put("run_number", RUN_ID);

    // Eventually, this test should make a mixed-up streams of
    // Fragments; this has too clean a pattern to be an interesting
    // test of the EventStore's ability to deal with multiple events
    // simulatenously.
    GenericFragmentSimulator sim(sim_config);
        
    EventStore events(NUM_FRAGS_PER_EVENT, RUN_ID, STORE_ID, argc, argv, &artapp);
    
    FragmentPtrs frags;
    while (sim.getNext(frags))
      {
        LOG_DEBUG("main") << "Number of fragments: " << frags.size() << '\n';

        assert(frags.size() == NUM_FRAGS_PER_EVENT);
        for (auto&& frag : frags) 
          {
            assert(frag != nullptr);
            events.insert(std::move(frag));
          }
        frags.clear();
      }

    rc = events.endOfData();
  }
  catch (cet::exception& x) {
    std::cerr << argv[0] << " failure\n" << x << std::endl;
    rc = 1;
  }
  catch (std::string& x) {
    std::cerr << argv[0] << " failure\n" << x << std::endl;
    rc = 2;
  }
  catch (char const* x) {
    std::cerr << argv[0] << " failure\n" << x << std::endl;
    rc = 3;
  }
  return rc;
}
