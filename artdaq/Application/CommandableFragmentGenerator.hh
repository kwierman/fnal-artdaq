#ifndef artdaq_Application_CommandableFragmentGenerator_hh
#define artdaq_Application_CommandableFragmentGenerator_hh

////////////////////////////////////////////////////////////////////////
// CommandableFragmentGenerator is a FragmentGenerator-derived
// abstract class that defines the interface for a FragmentGenerator
// designed as a state machine with start, stop, etc., transition
// commands. Users of classes derived from
// CommandableFragmentGenerator will call these transitions via the
// publically defined StartCmd(), StopCmd(), etc.; these public
// functions contain functionality considered properly universal to
// all CommandableFragmentGenerator-derived classes, including calls
// to private virtual functions meant to be overridden in derived
// classes. The same applies to this class's implementation of the
// FragmentGenerator::getNext() pure virtual function, which is
// declared final (i.e., non-overridable in derived classes) and which
// itself calls a pure virtual getNext_() function to be implemented
// in derived classes.

// State-machine related interface functions will be called only from a
// single thread. getNext() will be called only from a single
// thread. The thread from which state-machine interfaces functions are
// called may be a different thread from the one that calls getNext().
////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <mutex>

#include "fhiclcpp/fwd.h"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq/DAQdata/Fragments.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"


namespace artdaq {
  class CommandableFragmentGenerator : public FragmentGenerator {
  public:

    CommandableFragmentGenerator();
    CommandableFragmentGenerator(const fhicl::ParameterSet & );

    // Destroy the CommandableFragmentGenerator.
    virtual ~CommandableFragmentGenerator() = default;

    virtual bool getNext(FragmentPtrs & output) final;


    // John F., 12/11/13 -- not sure what the comment below means

    // This generator produces fragments with what distinct IDs (*not*
    // types)?
    virtual std::vector<Fragment::fragment_id_t> fragmentIDs() final;

    //
    // State-machine related interface below.
    //


    // After a call to 'start', all Fragments returned by getNext() will
    // be marked as part of a Run with the given run number, and with
    // subrun number 1. Calling start also resets the event number to 1.
    // After a call to start(), and until a call to stop, getNext() will
    // always return true, even if it returns no fragments.
    virtual void StartCmd(int run) final;

    // After a call to stop(), getNext() will eventually return
    // false. This may not happen for several calls, if the
    // implementation has data to be 'drained' from the system.
    virtual void StopCmd() final;

    // A call to pause() is advisory. It is an indication that the
    // BoardReader should stop the incoming flow of data, if it can
    // do so.
    virtual void PauseCmd() final;

    // After a call to resume(), the next Fragments returned from
    // getNext() will be part of a new SubRun.
    virtual void ResumeCmd() final;

    virtual std::string ReportCmd() final;

    // The following functions are not yet implemented, and their
    // signatures may be subject to change.

    // John F., 12/6/13 -- do we want Reset and Shutdown commands?

    //    virtual void ResetCmd() final {}
    //    virtual void ShutdownCmd() final {}

  protected:

    // John F., 12/6/13 -- need to figure out which of these getter
    // functions should be promoted to "public"

    int run_number() const { return run_number_; }
    int subrun_number() const { return subrun_number_; }
    bool should_stop() const { return should_stop_.load(); }
    bool exception() const { return exception_.load(); }

    int board_id () const { return board_id_; }
    int fragment_id () const { return fragment_id_; }
    size_t ev_counter () const { return ev_counter_.load (); }

    size_t ev_counter_inc (size_t step = 1) { return ev_counter_.fetch_add (step); } // returns the prev value

    void set_exception( bool exception ) { exception_.store( exception ); }

    // John F., 12/10/13 
    // Is there a better way to handle mutex_ than leaving it a protected variable?
    std::mutex mutex_;

  private:

    // In order to support the state-machine related behavior, all
    // CommandableFragmentGenerators must be able to remember a run number and a
    // subrun number.
    int run_number_, subrun_number_;

    std::atomic<bool> should_stop_, exception_;
    std::atomic<size_t> ev_counter_;

    int board_id_, fragment_id_;


    // Depending on what sleep_on_stop_us_ is set to, this gives the
    // stopping thread the chance to gather the required lock

    int sleep_on_stop_us_;

    // Obtain the next group of Fragments, if any are available. Return
    // false if no more data are available, if we are 'stopped', or if
    // we are not running in state-machine mode. Note that getNext_()
    // must return n of each fragmentID declared by fragmentIDs_().
    virtual bool getNext_(FragmentPtrs & output) = 0;

    // This generator produces fragments with what distinct IDs (*not*
    // types)?  Can be implemented using initializer syntax if
    // appropriate, e.g.  return { 3, 4 };
    virtual std::vector<Fragment::fragment_id_t> fragmentIDs_() = 0;

    //
    // State-machine related implementor interface below.
    //

    // If a CommandableFragmentGenerator subclass is reading from a file, and
    // start() is called, any run-, subrun-, and event-numbers in the
    // data read from the file must be over-written by the specified run
    // number, etc. After a call to start_(), and until a call to
    // stop_(), getNext_() is expected to return true.
    virtual void start() {}

    // If a CommandableFragmentGenerator subclass is reading from a file, calling
    // stop_() should arrange that the next call to getNext_() returns
    // false, rather than allowing getNext_() to read to the end of the
    // file.
    virtual void stop() {}

    // If a CommandableFragmentGenerator subclass is reading from hardware, the
    // implementation of pause_() should tell the hardware to stop
    // sending data.
    virtual void pause() {}

    // The subrun number will be incremented *before* a call to
    // resume. Subclasses are responsible for assuring that, after a
    // call to resume, that getNext_() will return Fragments marked with
    // the correct subrun number (and run number).
    virtual void resume() {}

    virtual std::string report() { return ""; }
  };

}

#endif /* artdaq_Application_CommandableFragmentGenerator_hh */
