/* DarkSide 50 DAQ program
 * This file add the xmlrpc commander as a client to the SC
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 */
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <stdexcept>
#include <iostream>
#include "xmlrpc_commander.hh"
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
        mf::LogDebug("xmlrpc_commander") << "Parameter list size = " << paramList.size();
        sleep(1);
        *retvalP = xmlrpc_c::value_string ("Success"); 
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("Command") << er.what ();
      } 
  };
  
  class start_: public cmd_ {
    public:
      start_ (xmlrpc_commander& c):
        cmd_(c, "s:i", "start the run") {}
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) try { 
        mf::LogDebug("xmlrpc_commander") << "Parameter list size = " << paramList.size();
        sleep(1);
        *retvalP = xmlrpc_c::value_string ("Success"); 
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("Command") << er.what ();
      } 
  };

#define generate_noarg_class(name, description) \
  class name ## _: public cmd_ { \
    public: \
      name ## _(xmlrpc_commander& c):\
          cmd_(c, "s:n", description) {}\
      void execute (xmlrpc_c::paramList const&, xmlrpc_c::value * const retvalP) try { \
        sleep(1); \
        *retvalP = xmlrpc_c::value_string ("Success");\
      } catch (std::runtime_error &er) { \
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); \
	mf::LogError ("Command") << er.what (); \
      } \
  } 

  generate_noarg_class(pause, "pause the run");
  generate_noarg_class(resume, "resume the run");
  generate_noarg_class(stop, "stop the run");

#undef generate_noarg_class

  class shutdown_: public xmlrpc_c::registry::shutdown {
    public:
      shutdown_ (xmlrpc_c::serverAbyss *server): _server(server) {}

      virtual void doit (const std::string& paramString, void*) const {
        mf::LogDebug("xmlrpc_commander") << paramString;
	_server->terminate ();
      }
    private:
      xmlrpc_c::serverAbyss *_server;
  };
}


xmlrpc_commander::xmlrpc_commander (int port): _port(port)
{}

void xmlrpc_commander::run() try {
  xmlrpc_c::registry registry;

#define register_method(m) \
  xmlrpc_c::methodPtr const ptr_ ## m(new m ## _(*this));\
  registry.addMethod ("ds50." #m, ptr_ ## m);

  register_method(init);
  register_method(start);
  register_method(stop);

#undef register_method

  xmlrpc_c::serverAbyss server(xmlrpc_c::serverAbyss::constrOpt ().registryP (&registry).portNumber (_port));

  shutdown_ shutdown_obj(&server);
  registry.setShutdown (&shutdown_obj);

  mf::LogDebug ("XMLRPC") << "running server" << std::endl;

  server.run();

  mf::LogDebug ("XMLRPC") << "server terminated" << std::endl;

} catch (...) {
  throw;
}
