#ifndef artdaq_DAQdata_FragmentGenerator_hh
#define artdaq_DAQdata_FragmentGenerator_hh

////////////////////////////////////////////////////////////////////////
// FragmentGenerator is an abstract class that defines the interface for
// obtaining events. Subclasses are to override the (private) virtual
// functions; users of FragmentGenerator are to invoke the public
// (non-virtual) functions.
//
// All classes that inherit from FragmentGenerator are required to
// implement the state-machine related functions. Some classes that
// inherit from FragmentGenerator can work only when invoked by a client
// that obeys the associated state-machine rules. Other classes that
// inherit from FragmentGenerator may also allow programs to call *none*
// of the state-machine related function; these programs will call only
// getNext().
//
// State-machine related interface functions will be called only from a
// single thread. getNext() will be called only from a single
// thread. The thread from which state-machine interfaces functions are
// called may be a different thread from the one that calls getNext().
////////////////////////////////////////////////////////////////////////

#include "fhiclcpp/fwd.h"
#include "artdaq/DAQdata/Fragments.hh"

namespace artdaq {
  class FragmentGenerator {
  public:

    // All derived classes will invoke the default constructor for the
    // base class. This can be done implicitly.
    FragmentGenerator() = default;

    // Destroy the FragmentGenerator.
    virtual ~FragmentGenerator() = default;

    // Obtain the next collection of Fragments. Return false to indicate
    // end-of-data. Fragments may or may not be in the same event;
    // Fragments may or may not have the same FragmentID. Fragments
    // will all be part of the same Run and SubRun.
    bool getNext(FragmentPtrs & output);


    // This generator produces fragments with what distinct IDs (*not*
    // types)?
    std::vector<Fragment::fragment_id_t> fragmentIDs();

    //
    // State-machine related interface below.
    //

    // Return true if we can only run under control of a state machine,
    // and false otherwise. Clients are *not* permitted to use only part
    // of the state-machine related behavior; if any of the
    // state-machine related functions are used, they must all be used
    // in a consistent fashion.
    bool requiresStateMachine() const;

    // After a call to 'start', all Fragments returned by getNext() will
    // be marked as part of a Run with the given run number, and with
    // subrun number 1. Calling start also resets the event number to 1.
    // After a call to start(), and until a call to stop, getNext() will
    // always return true, even if it returns no fragments.
    void start(int run);

    // After a call to stop(), getNext() will eventually return
    // false. This may not happen for several calls, if the
    // implementation has data to be 'drained' from the system.
    void stop();

    // A call to pause() is advisory. It is an indication that the
    // FragmentReceiver should stop the incoming flow of data, if it can
    // do so.
    void pause();

    // After a call to resume(), the next Fragments returned from
    // getNext() will be part of a new SubRun.
    void resume();

    // The following functions are not yet implemented, and their
    // signatures may be subject to change.

    // std::string report();
    // void reset();
    // void shutdown();

  protected:

    // Only derived classes can use these functions. Note that if a
    // particular instance of a subclass is not being used in the
    // state-machine mode, the values returned by these functions may be
    // invalid.
    int run_number() const;
    int subrun_number() const;

  private:

    // In order to support the state-machine related behavior, all
    // FragmentGenerators must be able to remember a run number and a
    // subrun number.
    int run_number_ = -1;    // default value is invalid.
    int subrun_number_ = -1; // default value is invalid.

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

    // Return true if clients *must* use the state-machine related
    // interface, and false if they are permitted to ignore the
    // state-machine related interface entirely.
    virtual bool requiresStateMachine_() const = 0;

    // If a FragmentGenerator subclass is reading from a file, and
    // start() is called, any run-, subrun-, and event-numbers in the
    // data read from the file must be over-written by the specified run
    // number, etc. After a call to start_(), and until a call to
    // stop_(), getNext_() is expected to return true.
    virtual void start_() = 0;

    // If a FragmentGenerator subclass is reading from a file, calling
    // stop_() should arrange that the next call to getNext_() returns
    // false, rather than allowing getNext_() to read to the end of the
    // file.
    virtual void stop_() = 0;

    // If a FragmentGenerator subclass is reading from hardware, the
    // implementation of pause_() should tell the hardware to stop
    // sending data.
    virtual void pause_() = 0;

    // The subrun number will be incremented *before* a call to
    // resume. Subclasses are responsible for assuring that, after a
    // call to resume, that getNext_() will return Fragments marked with
    // the correct subrun number (and run number).
    virtual void resume_() = 0;
  };

}

#endif /* artdaq_DAQdata_FragmentGenerator_hh */
