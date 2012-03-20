#include "artdaq/DAQrate/EventStore.hh"
#include "cetlib/exception.h"

#include <cstddef>
#include <iostream>
#include <string>

using artdaq::EventStore;
using std::size_t;

int main(int argc, char* argv[]) try {

  size_t const NUM_FRAGS_PER_EVENT = 5;
  EventStore::run_id_t const RUN_ID = 2112;

  EventStore events(NUM_FRAGS_PER_EVENT, RUN_ID, argc, argv);
  return 0;
 }
 catch (cet::exception& x) {
   std::cerr << argv[0] << " failure\n" << x << std::endl;
   return 1;
 }
 catch (std::string& x) {
   std::cerr << argv[0] << " failure\n" << x << std::endl;
   return 2;
 }
 catch (char const* x) {
   std::cerr << argv[0] << " failure\n" << x << std::endl;
   return 3;
 }
