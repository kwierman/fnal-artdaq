/* DarkSide 50 DAQ program
 * This file add the xmlrpc commander as a client to the SC
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 */
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <stdexcept>
#include <iostream>
#include "art/Persistency/Provenance/RunID.h"
#include "ds50daq/DAQ/xmlrpc_commander.hh"
#include "fhiclcpp/make_ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace {
  std::string exception_msg (const std::runtime_error &er) { 
    std::string msg(er.what ()); //std::string(er.what ()).substr (2);
    if (msg[msg.size() - 1] == '\n') msg.erase(msg.size() - 1);
    return msg;
  }

  class cmd_: public xmlrpc_c::method {
    public:
      cmd_ (xmlrpc_commander& c, const std::string& signature, const std::string& description): _c(c) { _signature = signature; _help = description; }
    protected:
      xmlrpc_commander& _c;
  };

  class init_: public cmd_ {
    public:
      init_ (xmlrpc_commander& c):
        cmd_(c, "s:s", "initialize the program") {}
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) try {
        if (paramList.size() > 0) {
          std::string configString = paramList.getString(0);
          fhicl::ParameterSet pset;
          fhicl::make_ParameterSet(configString, pset);
          if (_c._commandable.initialize(pset)) {
            *retvalP = xmlrpc_c::value_string ("Success"); 
          }
          else {
            std::string problemReport = _c._commandable.report("all");
            *retvalP = xmlrpc_c::value_string (problemReport); 
          }
        }
        else {
          *retvalP = xmlrpc_c::value_string ("The init message requires a single argument that is a string containing the initializtion ParameterSet."); 
        }
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("XMLRPC_Commander") << er.what ();
      } 
  };

  class start_: public cmd_ {
    public:
      start_ (xmlrpc_commander& c):
        cmd_(c, "s:i", "start the run") {}
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) try {
        if (paramList.size() > 0) {
          std::string run_number_string = paramList.getString(0);
          art::RunNumber_t run_number =
            boost::lexical_cast<art::RunNumber_t>(run_number_string);
          art::RunID run_id(run_number);
          if (_c._commandable.start(run_id)) {
            *retvalP = xmlrpc_c::value_string ("Success"); 
          }
          else {
            std::string problemReport = _c._commandable.report("all");
            *retvalP = xmlrpc_c::value_string (problemReport); 
          }
        }
        else {
          *retvalP = xmlrpc_c::value_string ("The start message requires the run number as an argument."); 
        }
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("XMLRPC_Commander") << er.what ();
      } 
  };

  class status_: public cmd_ {
    public:
      status_ (xmlrpc_commander& c):
        cmd_(c, "s:s", "report application state") {}
      void execute (xmlrpc_c::paramList const&, xmlrpc_c::value * const retvalP) try {
        *retvalP = xmlrpc_c::value_string (_c._commandable.status());
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("XMLRPC_Commander") << er.what ();
      } 
  };

  class report_: public cmd_ {
    public:
      report_ (xmlrpc_commander& c):
        cmd_(c, "s:s", "report statistics") {}
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) try {
        if (paramList.size() > 0) {
          std::string which = paramList.getString(0);
          *retvalP = xmlrpc_c::value_string (_c._commandable.report(which));
        }
        else {
          *retvalP = xmlrpc_c::value_string ("The report message requires a single argument that selects the type of statistics to be reported."); 
        }
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("XMLRPC_Commander") << er.what ();
      } 
  };

#define generate_noarg_class(name, description) \
  class name ## _: public cmd_ { \
    public: \
      name ## _(xmlrpc_commander& c):\
          cmd_(c, "s:n", description) {}\
      void execute (xmlrpc_c::paramList const&, xmlrpc_c::value * const retvalP) try { \
        if (_c._commandable.name()) { \
          *retvalP = xmlrpc_c::value_string ("Success"); \
        } \
        else { \
          std::string problemReport = _c._commandable.report("all"); \
          *retvalP = xmlrpc_c::value_string (problemReport); \
        } \
      } catch (std::runtime_error &er) { \
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); \
	mf::LogError ("XMLRPC_Commander") << er.what (); \
      } \
  } 

  generate_noarg_class(stop, "stop the run");
  generate_noarg_class(pause, "pause the run");
  generate_noarg_class(resume, "resume the run");

#undef generate_noarg_class

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
}


xmlrpc_commander::xmlrpc_commander (int port, ds50::Commandable& commandable):
  _port(port), _commandable(commandable)
{}

void xmlrpc_commander::run() try {
  xmlrpc_c::registry registry;

#define register_method(m) \
  xmlrpc_c::methodPtr const ptr_ ## m(new m ## _(*this));\
  registry.addMethod ("ds50." #m, ptr_ ## m);

  register_method(init);
  register_method(start);
  register_method(stop);
  register_method(pause);
  register_method(resume);
  register_method(status);
  register_method(report);

#undef register_method

  xmlrpc_c::serverAbyss server(xmlrpc_c::serverAbyss::constrOpt ().registryP (&registry).portNumber (_port));

  shutdown_ shutdown_obj(&server);
  registry.setShutdown (&shutdown_obj);

  mf::LogDebug ("XMLRPC_Commander") << "running server" << std::endl;

  server.run();

  mf::LogDebug ("XMLRPC_Commander") << "server terminated" << std::endl;

} catch (...) {
  throw;
}
