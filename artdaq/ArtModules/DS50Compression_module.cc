#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/EventID.h"
#include "artdaq/Compression/Encoder.hh"
#include "artdaq/Compression/Properties.hh"
#include "artdaq/Compression/SymTable.hh"
#include "artdaq/DAQdata/DS50Board.hh"
#include "artdaq/DAQdata/DS50CompressedEvent.hh"
#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/Fragments.hh"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "artdaq/DAQrate/quiet_mpi.hh"

#include <iostream>
#include <list>
#include <ostream>
#include <fstream>
#include <string>

namespace {

  int my_mpi_rank() {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    return myrank;
  }

  struct compression_record {
    art::EventID                    eid;
    artdaq::Fragment::fragment_id_t fid;
    std::size_t                     uncompressed_size;
    std::size_t                     compressed_size;

    compression_record(art::EventID e,
                       artdaq::Fragment const& uncomp,
                       ds50::DataVec const& comp) :
      eid(e), fid(uncomp.fragmentID()),
      uncompressed_size(uncomp.dataSize()),
      compressed_size(comp.size())
    { }
  };

  std::ostream& operator<< (std::ostream& os,
                            compression_record const& r) {
    os << r.eid.run() << ' ' << r.eid.subRun() << ' ' << r.eid.event() << ' '
       << r.fid << ' '
       << r.uncompressed_size << ' '
       << r.compressed_size;
    return os;
  }
}

namespace ds50 {

  class DS50Compression : public art::EDProducer {
    static int const MPI_NOT_USED = -1;
  public:
    explicit DS50Compression(fhicl::ParameterSet const & p);
    virtual ~DS50Compression() { }

    virtual void produce(art::Event & e);
    virtual void endSubRun(art::SubRun & sr);
    virtual void endRun(art::Run & r);

  private:
    std::string const raw_label_;
    std::string const table_file_;
    SymTable table_;
    Encoder encode_;

    int mpi_rank_;
    bool record_compression_;
    // we use a list to avoid timing glitches from a vector resizing.
    std::list<compression_record> records_;
  };

  static SymTable callReadTable(std::string const & fname)
  {
    SymTable t;
    readTable(fname.c_str(), t, Properties::count_max());
    return t;
  }

  DS50Compression::DS50Compression(fhicl::ParameterSet const & p)
    : raw_label_(p.get<std::string>("raw_label")),
      table_file_(p.get<std::string>("table_file")),
      table_(callReadTable(table_file_)),
      encode_(table_),
      mpi_rank_(my_mpi_rank()),
      record_compression_(p.get<bool>("record_compression", false)),
      records_()
  {
    produces<CompressedEvent>();
  }

  void DS50Compression::produce(art::Event & e)
  {
    art::Handle<artdaq::Fragments> handle;
    e.getByLabel(raw_label_, handle);
    std::auto_ptr<CompressedEvent> prod(new CompressedEvent(*handle));
    // handle->dataBegin(), handle->dataEnd()
    size_t const len = handle->size();
#pragma omp parallel for shared(prod, handle)
    for (size_t i = 0; i < len; ++i) {
      {
#ifndef NDEBUG
        mf::LogDebug("Loop")
          << "Attempting to compress fragment # "
          << i;
#endif
      }
      artdaq::Fragment const & frag = (*handle)[i];
      Board b(frag);
      // start of payload is the DS50 header
#ifndef NDEBUG
      b.checkADCData(); // Check data integrity.
#endif
      auto adc_start = b.dataBegin();
      auto adc_end   = b.dataEnd();
      prod->fragment(i).resize(frag.dataSize());
      reg_type bit_count  = encode_(adc_start, adc_end, prod->fragment(i));
      prod->fragment(i).resize(std::ceil(bit_count / (8.0 * sizeof(DataVec::value_type))));
      prod->setFragmentBitCount(i, bit_count);
    }

    if (record_compression_) {
      for (size_t i = 0; i < len; ++i) {
        records_.emplace_back(e.id(), (*handle)[i], prod->fragment(i));
      }
    }

    e.put(prod);
  }

  void DS50Compression::endSubRun(art::SubRun &) { }

  void DS50Compression::endRun(art::Run &) {
    std::string filename("compression_stats_");
    filename += std::to_string(mpi_rank_);
    filename += ".txt";
    std::cerr << "Attempting to open file: " << filename << std::endl;

    std::ofstream ofs(filename);
    if (!ofs) {
      std::cerr << "Failed to open the output file." << std::endl;
    }

    for (auto const& r : records_) {
      ofs << r << "\n";
    }
    ofs.close();
  }

  DEFINE_ART_MODULE(DS50Compression)
}

