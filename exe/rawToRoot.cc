//
// given a set of raw electronics data files, build a single root file
// with events that contain the vector of fragments.
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

#include "ds50daq/DAQ/DS50FragmentGenerator.hh"
#include "ds50daq/DAQ/V172xFileReader.hh"
#include "ds50daq/DAQ/Config.hh"

#include <signal.h>
#include <iostream>
#include <memory>
#include <utility>
#include <cstdio>

using namespace ds50;
using namespace std;
using namespace fhicl;
namespace  bpo = boost::program_options;

volatile int events_to_generate;
void sig_handler(int) {events_to_generate = -1;}


class FileReader
{
public:
  FileReader(string const& name, size_t fragment_number);
  bool getNext(Fragment& out, size_t event_number);
private:
  string name_;
  size_t fragment_;
  FILE* fd_;
};

FileReader(string const& name, size_t fragment_number):
  name_(name),
  fragment_(fragment_number),
  fd_(fopen(name.c_str(),"rb"))
  
{
  if(!fd) throw runtime_error(name);
}

bool FileReader::getNext(Fragment& out, size_t event_number)
{
  V172xFragment::Header head;
  size_t items = fread(&head,sizeof(head),1,fd_);

  if(items==0 && feof(fd_)) return false;
  if(items!=1) 
    throw runtime_error(name_ + " <- failed reading header from this thing");

  size_t head_size_words = sizeof(head) / sizeof(Fragment::value_type);
  size_t total_word_count = ceil(head.event_size()*V172xFragment::Header::size_words 
				 / (double)sizeof(Fragment::value_type));
  size_t total_int32_to_read = head.event_size() - sizeof(head)/4;

  Fragment frag(event_number, fragment_, Config::V1720_FRAGMENT_TYPE);
  frag.resize(total_word_count);
  memcpy(frag.dataBegin(),&head,sizeof(head));
  Fragment::value_type* dest = frag.dataBegin() + head_size_words;
  items = fread(dest, 4, total_int32_to_read);

  if(items==0 && feof(fd_)) return false;
  if(items!=total_int32_to_read) 
    throw runtime_error(name_ + " <- failed reading data from this thing");

  out = frag;
  return true;
}

class Readers
{
public:
  Readers(vector<string> const&  fnames);
  bool get(artdaq::EventStore& store);
private:
  vector<FileReader> readers_;
};

Reader::Reader(vector<string> const& fnames)
{
  readers_.reserve(fnames.size());
  for(size_t i=0; i<fnames.size(), ++i)
    {
      readers_.emplace_back( FileReader(fnames[i], i+1) );
    }
}
  
// this should just return the vector of fragments instead of putting
// things into the event store

bool Readers::get(artdaq::EventStore& store)
{
  vector<Fragment> frags(readers_.size());
  bool not_done = true;

  for(size_t i=0; i<readers_size() && not_done; ++i)
    {
      not_done = readers_[i].getNext(frags[i],event_id_);
    }

  if(not_done) { }

  ++event_id;
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
  std::vector<std::shared_ptr<V172xFileReader>> readers;
  
  artdaq::EventStore::run_id_t run_num = 
    top_level_pset.get<artdaq::EventStore::run_id_t>("run_number",2112);
  
  for(size_t fnum=0;fnum < fnames.size();++fnum)
    {
      /*
	static const char* psetModel[] = "
	bundle_of_junk:
	{
	generator_ds50: { fragment_id: %s }
	fileNames: [ "fileName%s.dat" ]
	maxEvents: -1
	}
	";
      */

      ParameterSet pf,pg;
      pg.put("fragment_id",fnum+1);
      pf.put("generator_ds50",pg);
      // pf.put("maxEvents",1);
      pf.put<std::vector<std::string>>("fileNames", std::vector<std::string> { fnames[fnum] });
      readers.emplace_back( new V172xFileReader(pf) );
      readers.back()->start(run_num);
    }

  artdaq::EventStore store(fnames.size(), run_num,
                           1, argc, argv,
                           &artapp,
                           true);

  // int events_to_generate = top_level_pset.get<int>("events_to_generate", 0);
  int event_count = 1;

  bool done=false;
  while(true||false) // what the hell is this for? well... it won't stop.
    {
      for(auto& r:readers)
	{
	  artdaq::FragmentPtrs frags;
	  
	  if(not r->getNext(frags))
	    {
	      done=true;
	      break;
	    }

	cout << "E=" << event_count << " size=" << frags.front()->size() 
             << " datasize=" << frags.front()->dataSize()
             << endl;

	  frags.front()->setSequenceID(event_count);
	  frags.front()->setUserType(Config::V1720_FRAGMENT_TYPE);
	  store.insert(std::move(frags.front()));	  
	}

      if(done) break;
      ++event_count;
    }

  for(auto& r:readers) { r->stop(); }
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
