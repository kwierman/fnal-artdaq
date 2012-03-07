#include "boost/program_options.hpp"

#include "DAQdata/Fragments.hh"
#include "DAQrate/EventStore.hh"
#include "DAQrate/DS50EventGenerator.hh"
#include "DAQrate/DS50EventReader.hh"
#include "DAQrate/DS50EventSimulator.hh"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "cetlib/filepath_maker.h"
#include "cetlib/container_algorithms.h"

#include <iostream>
#include <memory>
#include <utility>

using namespace std;
using namespace fhicl;
namespace  bpo = boost::program_options;

artdaq::DS50EventGenerator* make_generator(ParameterSet const& ps)
{
  if (ps.get<bool>("do_random"))
    return new artdaq::DS50EventSimulator(ps);
  else
    return new artdaq::DS50EventReader(ps);
}

int main(int argc, char* argv[]) try
  {
    std::ostringstream descstr;
    descstr << argv[0]
            << " <-c <config-file>> <other-options> [<source-file>]+";

    bpo::options_description desc(descstr.str());
    desc.add_options()
      ("config,c", bpo::value<std::string>(), "Configuration file.")
      ("help,h", "produce help message");
    bpo::variables_map vm;
    try {
      bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(), vm);
      bpo::notify(vm);
    }
    catch (bpo::error const & e) {
      std::cerr << "Exception from command line processing in " << argv[0]
                << ": " << e.what() << "\n";
      return -1;
    }

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }

    if (!vm.count("config")) {
      std::cerr << "Exception from command line processing in " << argv[0]
                << ": no configuration file given.\n"
                << "For usage and an options list, please do '"
                << argv[0] <<  " --help"
                << "'.\n";
      return 2;
    }


    ParameterSet pset;
    cet::filepath_lookup lookup_policy("FHICL_FILE_PATH");
    make_ParameterSet(vm["config"].as<std::string>(),
                      lookup_policy, pset);
    ParameterSet ds_pset = pset.get<ParameterSet>("ds50");

    std::unique_ptr<artdaq::DS50EventGenerator> const gen(make_generator(ds_pset));

    artdaq::EventStore store(ds_pset.get<int>("source_count"),
                             ds_pset.get<int>("run_number"),
                             argc, argv);

    // Read or generate fragments as rapidly as possible, and feed them
    // into the EventStore. The throughput resulting from this design
    // choice is likely to have the fragment reading (or generation)
    // speed as the limiting factor
    artdaq::FragmentPtrs frags;

    while (gen->getNext(frags))
      {
        for (auto& val : frags)
          {
            store.insert(std::move(val));
          }
      }

#if 0
    // buffering way
    vector<FragmentPtrs> event_buffer;
    int total_events = ds_pset.get<int>("total_events");
    FragmentPtrs event;

    while(gen->getNext(event) && total_events>0)
      {
        event_buffer.push_back(event);
        --total_events;
      }

    for_each(event_buffer.begin(),event_buffer.end(),
             [&](FragmentsPtrs& val) { /* go through each fragment and put it onto the store queue */ }
             );
#endif

    store.endOfData();
    return 0;
  }

 catch (std::string& x) {
   cerr << "Exception (type string) caught in driver: " << x << '\n';
   return 1;
 }

 catch (char const* m) {
   cerr << "Exception (type char const*) caught in driver: ";
   if (m)
     cerr << m;
   else
     cerr << "[the value was a null pointer, so no message is available]";
   cerr << '\n';
 }
