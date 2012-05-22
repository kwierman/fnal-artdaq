#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "artdaq/Compression/Decoder.hh"
#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymTable.hh"
#include "artdaq/DAQdata/DS50BoardWriter.hh"
#include "artdaq/DAQdata/DS50CompressedEvent.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cmath>
#include <iostream>
#include <string>

namespace ds50 {

  class DS50Decompression : public art::EDProducer {
  public:
    explicit DS50Decompression(fhicl::ParameterSet const & p);
    virtual ~DS50Decompression() { }

    virtual void produce(art::Event & e);
    virtual void endSubRun(art::SubRun & sr);
    virtual void endRun(art::Run & r);

  private:
    std::string compressed_label_;
    std::string table_file_;
    SymTable table_;
    Decoder decode_;
  };

  static SymTable readAndSortTable(std::string const & fname)
  {
    SymTable t;
    readTable(fname.c_str(), t, Properties::count_max());
    std::sort(t.begin(),
              t.end(),
              [&](SymCode const & a,
                  SymCode const & b) {
                return a.bit_count_ < b.bit_count_;
              });
    return t;
  }

  DS50Decompression::DS50Decompression(fhicl::ParameterSet const & p)
    : compressed_label_(p.get<std::string>("compressed_label")),
      table_file_(p.get<std::string>("table_file")),
      table_(readAndSortTable(table_file_)),
      decode_(table_)
  {
    produces<artdaq::Fragments>();
    std::cerr << "Hello from DS50Decompression\n";
  }

  void DS50Decompression::produce(art::Event & e)
  {
    art::Handle<CompressedEvent> handle;
    e.getByLabel(compressed_label_, handle);
    size_t len = handle->size();
    std::auto_ptr<artdaq::Fragments> prod(new artdaq::Fragments(len));
#pragma omp parallel for shared(len, prod, handle)
    for (size_t i = 0; i < len; ++i) {
#ifndef NDEBUG
      mf::LogDebug("Loop")
        << "Attempting to decompress fragment # "
        << i;
#endif
      artdaq::Fragment & newfrag = (*prod)[i];
      newfrag = (handle->headerOnlyFrag(i)); // Load in the header
      BoardWriter b(newfrag);
      b.resize(b.total_adc_values());
      auto size_check __attribute__((unused))
        (decode_(handle->fragmentBitCount(i),
                 handle->fragment(i).begin(),
                 b.dataBegin(),
                 b.dataEnd()));
      assert(size_check == b.total_adc_values());
    }
    e.put(prod);
  }

  void DS50Decompression::endSubRun(art::SubRun &) { }
  void DS50Decompression::endRun(art::Run &) { }

  DEFINE_ART_MODULE(DS50Decompression)
}

