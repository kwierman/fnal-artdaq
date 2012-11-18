#ifndef DS50FragmentGenerator_hh
#define DS50FragmentGenerator_hh

#include <mutex>
#include <condition_variable>

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
      void perfreset ();

      int run_number () const { return run_number_; }
  private:
      virtual void start_ () {}
      virtual void pause_ () {}
      virtual void resume_ () {}
      virtual void stop_ () {}
      virtual std::string report_ () { return ""; }
      virtual void perfreset_ () {}

      virtual bool getNext_ (artdaq::FragmentPtrs & output) final;
      virtual bool getNext__ (artdaq::FragmentPtrs & output) = 0;

      int run_number_;
      bool should_stop_;
      std::mutex mutex_;
      std::condition_variable stop_wait_;

      struct stats {
	bool should_stop;
	int run_number;
	int call_count;
	double avg_frag_size;
      } stats_;
     
  protected:
      bool should_stop ();
  };
}

#endif /* artdaq_DAQdata_FragmentGenerator_hh */
