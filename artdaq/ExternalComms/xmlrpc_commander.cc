/* DarkSide 50 DAQ program
 * This file add the xmlrpc commander as a client to the SC
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 */
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <xmlrpc-c/girerr.hpp>
#include <stdexcept>
#include <iostream>
#include <limits>
#include <memory>
#include "art/Persistency/Provenance/RunID.h"
#include "artdaq/ExternalComms/xmlrpc_commander.hh"
#include "fhiclcpp/make_ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace {
  std::string exception_msg (const std::runtime_error &er,
                             const std::string &helpText="execute request") { 
    std::string msg("Exception when trying to ");
    msg.append(helpText);
    msg.append(": ");
    msg.append(er.what ()); //std::string(er.what ()).substr (2);
    if (msg[msg.size() - 1] == '\n') msg.erase(msg.size() - 1);
    return msg;
  }
  std::string exception_msg (const art::Exception &er,
                             const std::string &helpText) { 
    std::string msg("Exception when trying to ");
    msg.append(helpText);
    msg.append(": ");
    msg.append(er.what ());
    if (msg[msg.size() - 1] == '\n') msg.erase(msg.size() - 1);
    return msg;
  }
  std::string exception_msg (const cet::exception &er,
                             const std::string &helpText) { 
    std::string msg("Exception when trying to ");
    msg.append(helpText);
    msg.append(": ");
    msg.append(er.what ());
    if (msg[msg.size() - 1] == '\n') msg.erase(msg.size() - 1);
    return msg;
  }
  std::string exception_msg (const std::string &erText,
                             const std::string &helpText) { 
    std::string msg("Exception when trying to ");
    msg.append(helpText);
    msg.append(": ");
    msg.append(erText);
    if (msg[msg.size() - 1] == '\n') msg.erase(msg.size() - 1);
    return msg;
  }

  class cmd_: public xmlrpc_c::method {
    public:
      cmd_ (xmlrpc_commander& c, const std::string& signature, const std::string& description): _c(c) { _signature = signature; _help = description; }
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) {
	std::unique_lock<std::mutex> lk(_c.mutex_, std::try_to_lock);
	if (lk.owns_lock ()) execute_ (paramList, retvalP);
	else *retvalP = xmlrpc_c::value_string ("busy");
      }
    protected:
      xmlrpc_commander& _c;
      virtual void execute_ (const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retvalP) = 0;
  };

  class john_: public xmlrpc_c::method {

    public:

    // Can't seem to initialize "_signature" and "_help" in the initialization list...
    john_ (xmlrpc_commander& c, 
	   const std::string& signature, 
	   const std::string& description): 
      _c(c) { _signature = signature; _help = description; }

    void execute (const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retvalP) final;

  protected:

    // Can I make this private??
    xmlrpc_commander& _c;
  
    // "execute_" is a wrapper function around the call to the
    // commandable object's transition function

    virtual bool execute_ (const xmlrpc_c::paramList& , xmlrpc_c::value* const retvalP ) = 0;

    template <typename T> T getParam(const xmlrpc_c::paramList& paramList, int index);

    template <typename T> T getParam(const xmlrpc_c::paramList& paramList, int index, T default_value ); 

  };

  // Users are only allowed to call getParam for predefined types; see
  // template specializations below this default function

  template <typename T>
  T john_::getParam(const xmlrpc_c::paramList&, int ) {
    throw cet::exception("john_") << "Error in john_::getParam(): value type not supported" << std::endl;
  }

  template <>
  uint64_t john_::getParam<uint64_t>(const xmlrpc_c::paramList& paramList, int index) {
    return boost::lexical_cast<uint64_t>(paramList.getInt(index));
  }

  template <>
  std::string john_::getParam<std::string>(const xmlrpc_c::paramList& paramList, int index) {
    return static_cast<std::string>( paramList.getString(index) );
  }

  template <>
  art::RunID john_::getParam<art::RunID>(const xmlrpc_c::paramList& paramList, int index) {

    std::string run_number_string = paramList.getString(index);
    art::RunNumber_t run_number =
      boost::lexical_cast<art::RunNumber_t>(run_number_string);
    art::RunID run_id(run_number);
    
    return run_id;
  }

  template <>
  fhicl::ParameterSet john_::getParam<fhicl::ParameterSet>(const xmlrpc_c::paramList& paramList, int index) {

    std::string configString = paramList.getString(index);
    fhicl::ParameterSet pset;
    fhicl::make_ParameterSet(configString, pset);

    return pset;
  }

  // Here, if getParam throws an exception due to a lack of an
  // existing parameter, swallow the exception and return the
  // default value passed to the function

  // Surpringly, if an invalid index is supplied, although getParam
  // throws an exception that exception is neither xmlrpc_c's
  // girerr:error nor boost::bad_lexical_cast. Although it's less
  // than ideal, we'll swallow all exceptions in the call to
  // getParam, as an invalid index value simply means the user wishes to employ the default_value

  template <typename T> T john_::getParam(const xmlrpc_c::paramList& paramList, int index,
					  T default_value ) {
    T val = default_value;
      
    try {
      val = getParam<T>(paramList, index);
    } catch (...) {
    }
    
    return val;
  }

  void john_::execute(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const retvalP) {

    std::unique_lock<std::mutex> lk(_c.mutex_, std::try_to_lock);
    if (lk.owns_lock ()) {
      try {

	// JCF, 9/4/14

	// Assuming the execute_ function returns true, then if the
	// retvalP argument was untouched, assign it the string
	// "Success"

	// See
	// http://xmlrpc-c.sourceforge.net/doc/libxmlrpc++.html#isinstantiated
	// for more on the concept of instantiation in xmlrpc_c::value objects

	if ( execute_(paramList, retvalP ) ) {
	  
	  if (! retvalP->isInstantiated() ) {
	    *retvalP = xmlrpc_c::value_string ("Success"); 
	  } 
	}
	else {
	  std::string problemReport = _c._commandable.report("all");
	  *retvalP = xmlrpc_c::value_string (problemReport); 
	}

      } catch (std::runtime_error &er) { 
	std::string msg = exception_msg (er, _help);
	*retvalP = xmlrpc_c::value_string (msg); 
	mf::LogError ("XMLRPC_Commander") << msg;
      } catch (art::Exception &er) { 
	std::string msg = exception_msg (er, _help);
	*retvalP = xmlrpc_c::value_string (msg); 
	mf::LogError ("XMLRPC_Commander") << msg;
      } catch (cet::exception &er) { 
	std::string msg = exception_msg (er, _help);
	*retvalP = xmlrpc_c::value_string (msg); 
	mf::LogError ("XMLRPC_Commander") << msg;
      } catch (...) { 
	std::string msg = exception_msg ("Unknown exception", _help);
	*retvalP = xmlrpc_c::value_string (msg); 
	mf::LogError ("XMLRPC_Commander") << msg;
      } 
    }
    else {
      *retvalP = xmlrpc_c::value_string ("busy");
    }

  }

  class init_: public john_ {

  public:
    init_(xmlrpc_commander& c):
      john_(c, "s:sii", "initialize the program") {}

    static const uint64_t defaultTimeout = 45;
    static const uint64_t defaultTimestamp = std::numeric_limits<const uint64_t>::max();

  private:
    bool execute_(const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const  ) {

      std::cout << "In init, passing: " << getParam<fhicl::ParameterSet>(paramList, 0) << ", " \
		<< getParam<uint64_t>(paramList, 1, defaultTimeout) << ", " \
		<< getParam<uint64_t>(paramList, 2, defaultTimestamp) << std::endl;
	
	

      return _c._commandable.initialize( getParam<fhicl::ParameterSet>(paramList, 0),
					 getParam<uint64_t>(paramList, 1, defaultTimeout),
					 getParam<uint64_t>(paramList, 2, defaultTimestamp)
					 );
    }
  };

  class soft_init_: public john_ {

  public:
    soft_init_ (xmlrpc_commander& c):
      john_(c, "s:sii", "initialize software components in the program") {}

    static const uint64_t defaultTimeout = 45;
    static const uint64_t defaultTimestamp = std::numeric_limits<const uint64_t>::max();

  private:
      bool execute_ (xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const  )  {
	return _c._commandable.soft_initialize( getParam<fhicl::ParameterSet>(paramList, 0),
						getParam<uint64_t>(paramList, 1, defaultTimeout),
						getParam<uint64_t>(paramList, 2, defaultTimestamp)
						); 
      }
  };

  class reinit_: public john_ {
  
  public:
      reinit_ (xmlrpc_commander& c):
        john_(c, "s:sii", "re-initialize the program") {}

    static const uint64_t defaultTimeout = 45;
    static const uint64_t defaultTimestamp = std::numeric_limits<const uint64_t>::max();

  private:
    bool execute_ (xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const  ) {
      return _c._commandable.reinitialize(getParam<fhicl::ParameterSet>(paramList, 0),
					  getParam<uint64_t>(paramList, 1, defaultTimeout),
					  getParam<uint64_t>(paramList, 2, defaultTimestamp)
					  );
    }
  };

  class start_: public john_ {

  public:
    start_ (xmlrpc_commander& c):
      john_(c, "s:iii", "start the run") {}
    
    static const uint64_t defaultTimeout = 45;
    static const uint64_t defaultTimestamp = std::numeric_limits<const uint64_t>::max();

  private:

    bool execute_ (xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const  ) {

	  return _c._commandable.start( getParam<art::RunID>(paramList, 0),
					getParam<uint64_t>(paramList, 1, defaultTimeout),
					getParam<uint64_t>(paramList, 2, defaultTimestamp)
					); 
    }
  };

  class status_ : public john_ {

  public:
    status_ (xmlrpc_commander& c):
      john_(c, "s:s", "report the current state") {}

  private:

    bool execute_ (xmlrpc_c::paramList const& , xmlrpc_c::value* const retvalP ) {

      *retvalP = xmlrpc_c::value_string (_c._commandable.status());
      return true;
    }
  };

  class report_ : public john_ {

  public:
    report_ (xmlrpc_commander& c):
      john_(c, "s:s", "report statistics") {}

  private:
    bool execute_ (xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP ) {

      *retvalP = xmlrpc_c::value_string( _c._commandable.report( getParam<std::string>(paramList, 0) ) );
      return true;
    }
  };

  class reset_stats_ : public john_ {

  public:
    reset_stats_ (xmlrpc_commander& c):
      john_(c, "s:s", "reset statistics") {}

  private:
    bool execute_ (xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const  ) {

      return _c._commandable.reset_stats( getParam<std::string>(paramList, 0) );
    }
  };

  class legal_commands_: public john_ {

  public:
    legal_commands_ (xmlrpc_commander& c):
      john_(c, "s:n", "return the currently legal commands") {}

  private:
    bool execute_ (xmlrpc_c::paramList const&, xmlrpc_c::value* const retvalP) {
      
      std::vector<std::string> cmdList = _c._commandable.legal_commands();
      std::string resultString;
      
      for (auto& cmd : cmdList) {
          resultString.append(cmd + " ");
          if (cmd == "shutdown") {
            resultString.append(" reset");
          }
        }
        *retvalP = xmlrpc_c::value_string (resultString);

	return true;
    }
  };


  class stop_: public john_ {

  public:
    stop_(xmlrpc_commander& c):
      john_(c, "s:ii", "stop the program") {}

    static const uint64_t defaultTimeout = 45;
    static const uint64_t defaultTimestamp = std::numeric_limits<const uint64_t>::max();

  private:

    bool execute_ (const xmlrpc_c::paramList& paramList , xmlrpc_c::value* const  ) {
      return _c._commandable.stop( getParam<uint64_t>(paramList, 0, defaultTimeout),
				   getParam<uint64_t>(paramList, 1, defaultTimestamp)
				   );
    }
  };



  class pause_: public john_ {

  public:
    pause_(xmlrpc_commander& c):
      john_(c, "s:ii", "pause the program") {}

    static const uint64_t defaultTimeout = 45;
    static const uint64_t defaultTimestamp = std::numeric_limits<const uint64_t>::max();

  private:

    bool execute_ (const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const  ) {
      return _c._commandable.pause( getParam<uint64_t>(paramList, 0, defaultTimeout),
				   getParam<uint64_t>(paramList, 1, defaultTimestamp)
				   );
    }
  };


  class resume_: public john_ {

  public:
    resume_(xmlrpc_commander& c):
      john_(c, "s:ii", "resume the program") {}

    static const uint64_t defaultTimeout = 45;
    static const uint64_t defaultTimestamp = std::numeric_limits<const uint64_t>::max();

  private:

    bool execute_ (const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const  ) {
      return _c._commandable.resume( getParam<uint64_t>(paramList, 0, defaultTimeout),
				   getParam<uint64_t>(paramList, 1, defaultTimestamp)
				   );
    }
  };

  class shutdown_: public john_ {

  public:
    shutdown_(xmlrpc_commander& c):
      john_(c, "s:i", "shutdown the program") {}

    static const uint64_t defaultTimeout = 45;

  private:
    
    bool execute_ (const xmlrpc_c::paramList& paramList, xmlrpc_c::value* const ) {
      return _c._commandable.shutdown( getParam<uint64_t>(paramList, 0, defaultTimeout) );
    }
  };


// JCF, 9/4/14

// Not sure if anyone was planning to resurrect this code by changing
// the preprocessor decision; as such, I'll leave it in...

#if 0
  class shutdown_: public xmlrpc_c::registry::shutdown {
    public:
      shutdown_ (xmlrpc_c::serverAbyss *server): _server(server) {}

      virtual void doit (const std::string& paramString, void*) const {
        mf::LogInfo("XMLRPC_Commander") << "A shutdown command was sent "
                                        << "with parameter "
                                        << paramString << "\"";
	_server->terminate ();
      }
    private:
      xmlrpc_c::serverAbyss *_server;
  };
#endif
}


xmlrpc_commander::xmlrpc_commander (int port, artdaq::Commandable& commandable):
  _port(port), _commandable(commandable)
{}

void xmlrpc_commander::run() try {
  xmlrpc_c::registry registry;

#define register_method(m) \
  xmlrpc_c::methodPtr const ptr_ ## m(new m ## _(*this));\
  registry.addMethod ("daq." #m, ptr_ ## m);

  register_method(init);
  register_method(soft_init);
  register_method(reinit);
  register_method(start);
  register_method(status);
  register_method(report);
  register_method(stop);
  register_method(pause);
  register_method(resume);
  register_method(reset_stats);
  register_method(legal_commands);

  register_method(shutdown);

  // alias "daq.reset" to the internal shutdown transition
  xmlrpc_c::methodPtr const ptr_reset(new shutdown_(*this));
  registry.addMethod ("daq.reset", ptr_reset);
 
#undef register_method

  xmlrpc_c::serverAbyss server(xmlrpc_c::serverAbyss::constrOpt ().registryP (&registry).portNumber (_port));

#if 0
  shutdown_ shutdown_obj(&server);
  registry.setShutdown (&shutdown_obj);
#endif

  mf::LogDebug ("XMLRPC_Commander") << "running server" << std::endl;

  server.run();

  mf::LogDebug ("XMLRPC_Commander") << "server terminated" << std::endl;

} catch (...) {
  throw;
}
