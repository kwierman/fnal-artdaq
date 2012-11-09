#ifndef DS50FragmentGenerator_hh
#define DS50FragmentGenerator_hh

#include <mutex>

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/FragmentGenerator.hh"

namespace ds50 {
  class DS50FragmentGenerator : public artdaq::FragmentGenerator {
    public:
      DS50FragmentGenerator();
      virtual ~DS50FragmentGenerator() noexcept = default;

      void start (int run);
      void stop (); 
      void pause ();
      void resume ();
      std::string report ();

      int run_number () const { return run_number_; }
  private:
      virtual void start_ () {}
      virtual void stop_ () {}
      virtual void pause_ () {}
      virtual void resume_ () {}
      virtual std::string report_ () { return ""; }

      virtual bool getNext_ (artdaq::FragmentPtrs & output) final;
      virtual bool getNext__ (artdaq::FragmentPtrs & output) = 0;

      int run_number_;
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
