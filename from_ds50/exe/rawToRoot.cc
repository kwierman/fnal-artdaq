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
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"

#include "boost/program_options.hpp"

#include "ds50daq/DAQ/Config.hh"
#include "ds50daq/DAQ/FileReader.hh"
#include "ds50daq/DAQ/V172xFragment.hh"
#include "ds50daq/DAQ/V172xFragmentWriter.hh"

#include <iostream>
#include <memory>
#include <random>
#include <utility>

using namespace ds50;
using namespace std;
using namespace fhicl;
namespace  bpo = boost::program_options;
using artdaq::Fragment;
using artdaq::FragmentPtr;
using artdaq::EventStore;

class Readers
{
public:
  // If reading simulated raw data that contains event lengths in bytes
  // (rather than 32-bit ints, which is what the DAQ writes), set
  // 'size_in_words' to true.  To generate simulated V1724 fragments (in
  // addition to the V1720 fragments read from the file), set
  // 'makeV1724' to true. 'seed' is used to seed the random number
  // generated, which is only used if 'makeV1724' is true.
  Readers(size_t num_events,
          vector<string> const&  fnames, bool size_in_words, bool makeV1724,
          V172xFragment::adc_type seed);

  // Read all the input files, stopping when the first becomes
  // exhausted.  All Fragments read (or created, if 'makeV1724()' is
  // true) are put into the EventStore.
  void run_to_end(EventStore& store);

private:
  // Read one Fragment from each file, passing each to the
  // EventStore. Return false when the first file becomes exhausted.
  bool handle_next_event_(size_t eid, EventStore& store);

  // Insert a V1724 fragment into 'store'; the V1724 fragment is made
  // based upon the given V1720 fragment by shifting the data by two
  // bits, and adding two bits of random noise.
  void insertV1724(Fragment const& v1720, EventStore& store);

  size_t num_events_;
  bool makeV1724_;
  size_t numV1720fragmentsPerEvent_;
  vector<FileReader> readers_;
  std::independent_bits_engine<std::minstd_rand, 2, V172xFragment::adc_type> twoBits_;
};

Readers::Readers(size_t num_events,
                 vector<string> const& fnames, bool size_in_words,
                 bool makeV1724, V172xFragment::adc_type seed) :
  num_events_(num_events),
  makeV1724_(makeV1724),
  numV1720fragmentsPerEvent_(fnames.size()),
  readers_(),
  twoBits_(seed)
{
  readers_.reserve(numV1720fragmentsPerEvent_);
  for (size_t i=0; i < numV1720fragmentsPerEvent_; ++i)
    readers_.emplace_back(fnames[i], i+1, size_in_words);
}

void Readers::run_to_end(EventStore& store)
{
  size_t events_read = 0;
  if (num_events_ == 0) num_events_ = -1;
  while (true && events_read < num_events_)
    {
      bool rc = handle_next_event_(events_read+1, store);
      if (!rc) break;
      ++events_read;
    }
}

bool Readers::handle_next_event_(size_t event_num,
                                 EventStore& store)
{
  for (auto& r : readers_)
    {
      FragmentPtr fp = r.getNext(event_num);
      if (fp == nullptr) return false;
      if (makeV1724_) insertV1724(*fp, store);
      store.insert(move(fp));
    }
  return true;
}

void

Readers::insertV1724(Fragment const& v1720, EventStore& store)
{
  FragmentPtr v1724(new Fragment(v1720));
  v1724->setUserType(Config::V1724_FRAGMENT_TYPE);
  v1724->setFragmentID(v1720.fragmentID() + numV1720fragmentsPerEvent_);

  V172xFragmentWriter overlay(*v1724);
  for (auto i = overlay.dataBegin(), e = overlay.dataEnd(); i != e; ++i)
    {
      unsigned short tmp = (*i) << 2;
      tmp |= twoBits_();
      *i = tmp;

    }
  store.insert(std::move(v1724));
}


//-----------------------------------------------------------------------------

int main(int argc, char * argv[]) try
{
  ostringstream descstr;
  descstr << argv[0] << " <-c <config-file>> <other-options>";
  bpo::options_description desc(descstr.str());
  desc.add_options()
    ("config,c", bpo::value<string>(), "Configuration file.")
    ("help,h", "produce help message");
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(), vm);
    bpo::notify(vm);
  }
  catch (bpo::error const & e) {
    cerr << "Exception from command line processing in " << argv[0]
              << ": " << e.what() << "\n";
    return -1;
  }
  if (vm.count("help")) {
    cout << desc << endl;
    return 1;
  }
  if (!vm.count("config")) {
    cerr << "Exception from command line processing in " << argv[0]
              << ": no configuration file given.\n"
              << "For usage and an options list, please do '"
              << argv[0] <<  " --help"
              << "'.\n";
    return 2;
  }

  if (getenv("FHICL_FILE_PATH") == nullptr)
    {
      cerr
        << "INFO: environment variable FHICL_FILE_PATH was not set. Using \".\"\n";
      setenv("FHICL_FILE_PATH", ".", 0);
    }
  cet::filepath_lookup_after1 lookup_policy("FHICL_FILE_PATH");

  ParameterSet top_level_pset;
  make_ParameterSet(vm["config"].as<string>(), lookup_policy, top_level_pset);

  vector<string> fnames =
    top_level_pset.get<vector<string>>("file_names");

  auto const num_events = top_level_pset.get<size_t>("num_events", 0);
  bool const size_in_words = top_level_pset.get<bool>("size_in_words", true);
  bool const makeV1724 = top_level_pset.get<bool>("makeV1724", true);
  auto const seed = top_level_pset.get<V172xFragment::adc_type>("seed", 30031);
  Readers readers(num_events, fnames, size_in_words, makeV1724, seed);


  EventStore::run_id_t run_num =

    top_level_pset.get<EventStore::run_id_t>("run_number", 1);

  // If we are generating V1724 fragments in addition to reading the
  // V1720 fragments, then we make twice as many fragments as files.
  size_t num_frags = makeV1724 ? fnames.size() * 2 : fnames.size();


  EventStore store(num_frags, run_num, 1, argc, argv, &artapp, true);

  readers.run_to_end(store);
  return store.endOfData();
 }

catch (string & x)
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
