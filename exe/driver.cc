//
// Driver is a program for testing the behavior of the generic
// RawInput source. Run 'driver --help' to get a description of the
// expected command-line parameters.
//
//
// The current version generates simple data fragments, for testing
// that data are transmitted without corruption from the
// artdaq::EventStore through to the artdaq::RawInput source.
//

#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQdata/FragmentGenerator.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/GenericFragmentSimulator.hh"
#include "artdaq/DAQdata/makeFragmentGenerator.hh"
#include "artdaq/DAQrate/EventStore.hh"
#include "artdaq/DAQrate/SimpleQueueReader.hh"
#include "cetlib/container_algorithms.h"
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"

#include "boost/program_options.hpp"

#include <iostream>
#include <memory>
#include <utility>

using namespace std;
using namespace fhicl;
namespace  bpo = boost::program_options;

int main(int argc, char * argv[]) try
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
  ParameterSet driver_pset = pset.get<ParameterSet>("driver");
  std::unique_ptr<artdaq::FragmentGenerator> const
    gen(artdaq::makeFragmentGenerator(driver_pset.get<std::string>("generator"),
                                      driver_pset));
  artdaq::FragmentPtrs frags;
  while (gen->getNext(frags)) {}
  //////////////////////////////////////////////////////////////////////
  // Note: we are constrained to doing all this here rather than
  // encapsulated neatly in a function due to the lieftime issues
  // associated with async threads and std::string::c_str().
  bool const want_artapp(driver_pset.get<bool>("use_art", false));
  std::ostringstream os;
  if (!want_artapp) {
    os << pset.get<int>("events_expected");
  }
  std::string const oss(os.str());
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
  char * args[2] {"SimpleQueueReader", const_cast<char *>(oss.c_str())};
#pragma GCC diagnostic pop
  int es_argc(want_artapp?argc:2);
  char **es_argv (want_artapp?argv:args);
  artdaq::EventStore::ARTFUL_FCN *
    es_fcn(want_artapp?&artapp:&artdaq::simpleQueueReaderApp);
  artdaq::EventStore store(driver_pset.get<size_t>("source_count"),
                           driver_pset.get<artdaq::EventStore::run_id_t>("run_number"),
                           1,
                           es_argc,
                           es_argv,
                           es_fcn);
  //////////////////////////////////////////////////////////////////////

  // Read or generate fragments as rapidly as possible, and feed them
  // into the EventStore. The throughput resulting from this design
  // choice is likely to have the fragment reading (or generation)
  // speed as the limiting factor
  for (auto & val : frags) {
    store.insert(std::move(val));
  }
  return store.endOfData();
}

catch (std::string & x)
{
  cerr << "Exception (type string) caught in driver: " << x << '\n';
  return 1;
}

catch (char const * m)
{
  cerr << "Exception (type char const*) caught in driver: ";
  if (m)
  { cerr << m; }
  else
  { cerr << "[the value was a null pointer, so no message is available]"; }
  cerr << '\n';
}
