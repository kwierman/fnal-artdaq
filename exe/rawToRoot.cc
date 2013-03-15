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
  Readers(vector<string> const&  fnames);
  bool run_to_end(artdaq::EventStore& store);
private:
  vector<FileReader> readers_;
};

Readers::Readers(vector<string> const& fnames) :
  readers_()
{
  readers_.reserve(fnames.size());
  for (size_t i=0, sz=fnames.size(); i < sz; ++i)
    readers_.emplace_back(fnames[i], i+1);
}
  
bool Readers::run_to_end(artdaq::EventStore& store)
{
  bool keep_going = true;
  for (auto& r : readers_)
    {
      artdaq::Fragment frag;
      keep_going &&= r.getNext(frag);
      if (!keep_going) break;
      store.insert(frag);
    }

  return keep_going;
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
  Readers readers(fnames);
  
  artdaq::EventStore::run_id_t run_num = 
    top_level_pset.get<artdaq::EventStore::run_id_t>("run_number",2112);
  
  // for (size_t fnum = 0, sz=fnames.size(); fnum < sz; ++fnum)
  //   {
  //     ParameterSet pf, pg;
  //     pg.put("fragment_id", fnum+1);
  //     pf.put("generator_ds50", pg);
  //     pf.put<std::vector<std::string>>("fileNames", 
  //                                      std::vector<std::string> { fnames[fnum] });
  //     readers.emplace_back( new V172xFileReader(pf) );
  //     readers.back()->start(run_num);
  //   }

  artdaq::EventStore store(fnames.size(), run_num, 1, argc, argv, &artapp, true);

  // int event_count = 1;
  // bool done=false;

  // while (true)
  //   {
      
  //     for(auto& r:readers)
  //       {
  //         artdaq::FragmentPtrs frags;
	  
  //         if(not r->getNext(frags))
  //           {
  //             done=true;
  //             break;
  //           }

  //         cout << "E=" << event_count << " size=" << frags.front()->size() 
  //              << " datasize=" << frags.front()->dataSize()
  //              << endl;

  //         frags.front()->setSequenceID(event_count);
  //         frags.front()->setUserType(Config::V1720_FRAGMENT_TYPE);
  //         store.insert(std::move(frags.front()));	  
  //       }

  //     if(done) break;
  //     ++event_count;
  //   }

  // for(auto& r:readers) { r->stop(); }

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
