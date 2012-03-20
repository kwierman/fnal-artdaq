
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Provenance/BranchType.h"

#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymTable.hh"
#include "artdaq/Compression/Encoder.hh"
#include "artdaq/DAQdata/DS50RawData.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/DS50Board.hh"

#include <iostream>
#include <string>

namespace ds50
{

  class DS50Compression : public art::EDProducer
  {
  public:
    explicit DS50Compression( fhicl::ParameterSet const& p );
    virtual ~DS50Compression() { }
    
    virtual void produce(art::Event& e);
    virtual void endSubRun(art::SubRun& sr);
    virtual void endRun(art::Run& r);

  private:
    std::string raw_label_;
    std::string table_file_;
    SymTable table_;
    Encoder encode_;
  };

  static SymTable callReadTable(std::string const& fname)
  {
    SymTable t;
    readTable(fname.c_str(),t,Properties::count_max());
    return t;
  }

  DS50Compression::DS50Compression( fhicl::ParameterSet const& p )
    : raw_label_( p.get<std::string>("raw_label") ),
      table_file_(p.get<std::string>("table_file") ),
      table_(callReadTable(table_file_)),
      encode_(table_)
  {
    produces<DS50RawData>();
  }

  void DS50Compression::produce(art::Event& e)
  {
    typedef std::vector<artdaq::Fragment> FragVec;

    art::Handle<FragVec> handle;
    e.getByLabel(raw_label_, handle);

    std::auto_ptr<DS50RawData> prod(new DS50RawData(*handle));

    // handle->dataBegin(), handle->dataEnd()
    size_t len = handle->size();
    for(size_t i=0;i<len;++i)
      {
	auto payload = (*handle)[i].dataBegin();
	// start of payload is the DS50 header
	Board b((*handle)[i]);
	// DS50 data immediately follows the header
	// the board class should be giving out the start/end of
	// data as adc_type pointer instead of calculating it here
	// DS50 data is in data_t units.
	ds50::Board::data_t* head_start = (ds50::Board::data_t*)*payload;
	ds50::Board::data_t* data_start = head_start + b.header_size_words();
	// we want the data in adc_type units
	// there are two adc values per word
	adc_type* adc_start = (adc_type*)data_start;
	adc_type* adc_end   = adc_start + (b.event_size() * 2);

	reg_type bit_count  = encode_(adc_start,adc_end, prod->fragment(i));
	prod->setFragmentBitCount(i,bit_count);
      }

    e.put(prod);
  }

  void DS50Compression::endSubRun(art::SubRun&) { }
  void DS50Compression::endRun(art::Run&) { }

  DEFINE_ART_MODULE(DS50Compression)
}

