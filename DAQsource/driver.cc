

#include "boost/program_options.hpp"

#include "DAQdata/RawData.hh"
#include "DAQrate/EventStore.hh"
//#include "DAQrate/DS50Reader.hh"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "cetlib/filepath_maker.h"

#include <iostream>

using namespace std;
using namespace fhicl;
namespace  bpo = boost::program_options;

int main(int argc, char* argv[])
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

#if 0
  artdaq::DS50EventReader reader(ds_pset);

  artdaq::EventStore store(ds_pset.get<int>("run"),
			   ds_pset.get<int>("source_count"),
			   argc,argv);

  // simple way - speed depends on I/I reading
  Fragments el;
  while(reader.getNext(el))
    {
      for_each(el.begin(),el.end(),
	       [&](Fragments::value_type& val) { store.insert(val); }
	       );
    }

  // buffering way
  vector<Fragments> event_buffer;
  int total_events = ds_pset.get<int>("total_events");
  Fragments event;

  while(reader.getNext(event) && total_events>0)
    {
      event_buffer.push_back(event);
      --total_events;
    }

  for_each(event_buffer.begin(),event_buffer.end(),
	   [&](Fragments& val) { /* go through each fragment and put it onto the store queue */ }
	   );

#endif
  return 0;
}
