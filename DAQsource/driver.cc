
#include "DAQdata/RawData.hh"
#include "DAQrate/EventStore.hh"
#include "DAQrate/DS50Reader.hh"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "cetlib/filepath_maker.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
  if(argc<2)
    {
      cerr << "Usage: " << argv[0] << " fhicl_cntl_file\n";
      return -1;
    }

  fhicl::ParameterSet pset;
  cet::filepath_lookup gunk("FHICL_FILE_PATH");
  fhicl::make_ParameterSet(argv[1],gunk,pset);
  fhicl::ParameterSet ds_pset = pset.getParameterSet("ds50");

  artdaq::DS50Reader reader(ds_pset);

  artdaq::EventStore store(ds_pset.get<int>("run"),
			   ds_pset.get<int>("source_count"),
			   argc,argv);

  // reader.run(store);

  return 0;
}
