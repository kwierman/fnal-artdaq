
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

  return 0;
}
