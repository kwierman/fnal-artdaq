#define BOOST_TEST_MODULE ( GenericFragmentSimulator_t )
#include "boost/test/auto_unit_test.hpp"

#include "fhiclcpp/ParameterSet.h"
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/detail/RawFragmentHeader.hh"
#include "artdaq/DAQrate/EventStore.hh"
#include "artdaq/DAQrate/GenericFragmentSimulator.hh"

#include <cstddef>

std::size_t const NUM_EVENTS = 2;
std::size_t const NUM_FRAGS_PER_EVENT = 5;
std::size_t const FRAGMENT_SIZE = 110;
artdaq::EventStore::run_id_t const RUN_ID = 2112;

BOOST_AUTO_TEST_SUITE(FragmentGenerator_t)

BOOST_AUTO_TEST_CASE(Simple)
{
  fhicl::ParameterSet sim_config;
  sim_config.put("events_to_generate", NUM_EVENTS);
  sim_config.put("fragments_per_event", NUM_FRAGS_PER_EVENT);
  sim_config.put("run_number", RUN_ID);
  sim_config.put("want_random_fragment_size", false);
  sim_config.put("fragment_size", FRAGMENT_SIZE);
  artdaq::GenericFragmentSimulator sim(sim_config);
  artdaq::FragmentPtrs fragments;
  std::size_t num_events_seen = 0;
  while (fragments.clear(), sim.getNext(fragments)) {
    ++num_events_seen;
    BOOST_REQUIRE_EQUAL(fragments.size(), NUM_FRAGS_PER_EVENT);
  for (auto && fragptr : fragments) {
      BOOST_CHECK(fragptr.get());
      BOOST_CHECK_EQUAL(fragptr->sequenceID(), num_events_seen);
      BOOST_CHECK_EQUAL(fragptr->size(), FRAGMENT_SIZE + artdaq::detail::RawFragmentHeader::num_words());
      BOOST_CHECK_EQUAL(fragptr->dataSize(), FRAGMENT_SIZE);
    }
  }
  BOOST_REQUIRE_EQUAL(num_events_seen, NUM_EVENTS);
}

BOOST_AUTO_TEST_SUITE_END()
