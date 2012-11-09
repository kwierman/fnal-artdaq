#ifndef DS50FragmentGenerator_hh
#define DS50FragmentGenerator_hh

#include <mutex>

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/FragmentGenerator.hh"

namespace ds50 {
  class DS50FragmentGenerator : public artdaq::FragmentGenerator {
    public:
      virtual ~DS50FragmentGenerator() noexcept = default;

      bool start ();
      bool stop (); 
      bool pause ();
      bool resume ();
      std::string report ();

  private:
      virtual bool start_ () { return true; }
      virtual bool stop_ () { return true; } 
      virtual bool pause_ () { return true; }
      virtual bool resume_ () { return true; }
      virtual std::string report_ () { return ""; }

      virtual bool getNext_ (artdaq::FragmentPtrs & output) final;
      virtual bool getNext__ (artdaq::FragmentPtrs & output) = 0;
  protected:
      std::mutex mutex_;
      enum request {
	stop_r,
        pause_r,
        resume_r,
      } request_;
  };
}

#endif /* artdaq_DAQdata_FragmentGenerator_hh */
