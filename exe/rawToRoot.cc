//
// given a set of raw electronics data files, build a single root file
// with events that contain the vector of fragments.
//


#include "art/Framework/Art/artapp.h"
#include "artdaq/DAQdata/Fragment.hh"
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

#include "ds50daq/DAQ/Config.hh"
#include "ds50daq/DAQ/FileReader.hh"

#include <iostream>
#include <memory>
#include <utility>
#include <cstdio>

using namespace ds50;
using namespace std;
using namespace fhicl;
namespace  bpo = boost::program_options;

volatile int events_to_generate;

class Readers
{
public:
  Readers(vector<string> const&  fnames, bool size_in_words);
  // Read all the input files, stopping when the first becomes exhausted.
  // All Fragments read are put into the EventStore.
  void run_to_end(artdaq::EventStore& store);

private:
  // Read one Fragment from each file, passing each to the
  // EventStore. Return false when the first file becomes exhausted.
  bool handle_next_event_(size_t eid, artdaq::EventStore& store);
  vector<FileReader> readers_;
};

Readers::Readers(vector<string> const& fnames, bool size_in_words) :
  readers_()
{
  readers_.reserve(fnames.size());
  for (size_t i=0, sz=fnames.size(); i < sz; ++i)
    readers_.emplace_back(fnames[i], i+1, size_in_words);
}
  
void Readers::run_to_end(artdaq::EventStore& store)
{
  size_t events_read = 0;
  while (true)
    {
      bool rc = handle_next_event_(events_read+1, store);
      if (!rc) break;
      ++events_read;
    }
}

bool Readers::handle_next_event_(size_t event_num,
                                 artdaq::EventStore& store)
{
  for (auto& r : readers_)
    {
      std::unique_ptr<artdaq::Fragment> fp = r.getNext(event_num);
      if (fp == nullptr) return false;
      store.insert(std::move(fp));
    }
  return true;
}

int main(int argc, char * argv[]) try
{
  std::ostringstream descstr;
  descstr << argv[0] << " <-c <config-file>> <other-options>";
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

  if (getenv("FHICL_FILE_PATH") == nullptr)
    {
      std::cerr
        << "INFO: environment variable FHICL_FILE_PATH was not set. Using \".\"\n";
      setenv("FHICL_FILE_PATH", ".", 0);
    }
  cet::filepath_lookup_after1 lookup_policy("FHICL_FILE_PATH");

  ParameterSet top_level_pset;
  make_ParameterSet(vm["config"].as<std::string>(), lookup_policy, top_level_pset);

  std::vector<std::string> fnames = 
    top_level_pset.get<std::vector<std::string>>("file_names");
  bool const size_in_words = false; // we read only the malformed file for now.
  Readers readers(fnames, size_in_words);
  
  artdaq::EventStore::run_id_t run_num = 
    top_level_pset.get<artdaq::EventStore::run_id_t>("run_number", 1);
  
  artdaq::EventStore store(fnames.size(), run_num, 1, argc, argv, &artapp, true);

  readers.run_to_end(store);
  return store.endOfData();
 }

catch (std::string & x)
{
  cerr << "Exception (type string) caught in ds50driver: " << x << '\n';
  return 1;
}

catch (char const * m)
{
  cerr << "Exception (type char const*) caught in ds50driver: ";
  if (m)
  { cerr << m; }
  else
  { cerr << "[the value was a null pointer, so no message is available]"; }
  cerr << '\n';
}
