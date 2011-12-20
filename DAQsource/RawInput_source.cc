
#include "DAQrate/ConcurrentQueue.hh"
#include "DAQrate/GlobalQueue.hh"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/ReaderSource.h"
#include "art/Framework/IO/Sources/put_product_in_principal.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

using std::string;

namespace artdaq
{
  struct EventStoreReader
  {
    art::PrincipalMaker const& pm_;
    RawEventQueue& queue_;

    EventStoreReader(fhicl::ParameterSet const& ps,
		     art::ProductRegistryHelper& help,
		     art::PrincipalMaker const& pm);

    void closeCurrentFile();
    void readFile(string const& name, art::FileBlock*& fb);
    
    bool readNext(art::RunPrincipal* const& inR,
                  art::SubRunPrincipal* const& inSR,
                  art::RunPrincipal*& outR,
                  art::SubRunPrincipal*& outSR,
                  art::EventPrincipal*& outE);
  };

  EventStoreReader::EventStoreReader(fhicl::ParameterSet const& ps,
				     art::ProductRegistryHelper& help,
				     art::PrincipalMaker const& pm):
    pm_(pm),
    queue_(getQueue())
  {
    const vector<string> inst_names = ps.get<vector<string>>("instances");
    for_each(inst_names.cbegin(),inst_names.cend(),
	     [&](string const& iname)
	     {
	       help.reconstitutes<Fragment, art::InEvent>("RawInput",iname);
	     });

    
  }

  void EventStoreReader::closeCurrentFile()
  {
  }
  
  void EventStoreReader::readFile(string const& name, 
				  art::FileBlock*& fb)
  {
    fb = new FileBlock(FileFormatVersion(1,"RawEvent2011"),"nothing");
  }
    
  bool EventStreReader::readNext(art::RunPrincipal* const& inR,
				 art::SubRunPrincipal* const& inSR,
				 art::RunPrincipal*& outR,
				 art::SubRunPrincipal*& outSR,
				 art::EventPrincipal*& outE)
  {
    RawEvent_ptr p;
    queue_.deqWait(p);

    if(!p)
      {
	// end of data stream
	return false;
      }

    Timestamp runstart;

    if(inR==0) outR=pm_.makeRunPrincipal();
    if(inSR==0) outSR=pm_.makeSubRunPrincipal();

    outE = pm_.makeEventPrincipal(inR==0?outR->run():inR->run(),
				  inSR==0?outSR->subRun():inSR->subRun(),
				  p->header_.event_id_,
				  runstart);

    // add all the fragments as products
    
  }

  typedef art::ReaderSource<EventStoreReader> RawInput;
}

DEFINE_ART_INPUT_SOURCE(artdaq::RawInput)
