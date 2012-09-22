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
      init_ (xmlrpc_commander& c): cmd_(c, "s:s", "initialize the system") {}
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) try {
	_c.init (paramList.getString (0));
	*retvalP = xmlrpc_c::value_string ("ok"); 
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("Command") << er.what ();
      } 
  };
  
  class start_: public cmd_ {
    public:
      start_ (xmlrpc_commander& c): cmd_(c, "s:i", "start the run") {}
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) try { 
	_c.start (paramList.getInt (0));
	*retvalP = xmlrpc_c::value_string ("ok"); 
      } catch (std::runtime_error &er) { 
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	mf::LogError ("Command") << er.what ();
      } 
  };

#define generate_noarg_class(name, description) \
  class name ## _: public cmd_ { \
    public: \
      name ## _(xmlrpc_commander& c): cmd_(c, "s:n", description) {} \
      void execute (xmlrpc_c::paramList const&, xmlrpc_c::value * const retvalP) try { \
	_c.name (); \
	*retvalP = xmlrpc_c::value_string ("ok"); \
      } catch (std::runtime_error &er) { \
	*retvalP = xmlrpc_c::value_string (exception_msg (er)); \
	mf::LogError ("Command") << er.what (); \
      } \
  } 

  generate_noarg_class(pause, "pause the run");
  generate_noarg_class(resume, "resume the run");
  generate_noarg_class(stop, "stop the run");
  generate_noarg_class(abort, "abort the system");
  generate_noarg_class(reboot, "reboot the computer");

#undef generate_noarg_class

  class shutdown_: public xmlrpc_c::registry::shutdown {
    public:
      shutdown_ (xmlrpc_c::serverAbyss *server): _server(server) {}

      virtual void doit (const std::string&, void*) const {
	_server->terminate ();
      }
    private:
      xmlrpc_c::serverAbyss *_server;
  };
}


xmlrpc_commander::xmlrpc_commander (int port): _port(port), _state(idle) {}

void xmlrpc_commander::operator() () try {
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

  shutdown ();
} catch (std::exception const& e) {
  std::cerr << "xmlrpc error " << e.what() << std::endl;
}



void xmlrpc_commander::init (const std::string& config) {
  std::lock_guard<std::mutex> lk(_m);
  if (_state != idle) throw std::runtime_error("wrong state");

  fhicl::make_ParameterSet (config, _pset);


  _state = inited;
}


void xmlrpc_commander::start (int) {
  std::lock_guard<std::mutex> lk(_m);
  if (_state != inited) throw std::runtime_error("wrong state");

  _state = running;
}


void xmlrpc_commander::stop () {
  if (_state != running) throw std::runtime_error("wrong state");
}

void xmlrpc_commander::shutdown () {
}
