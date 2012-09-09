/* DarkSide 50 DAQ program
 * This file add the xmlrpc commander as a client to the SC; the following
 * commands are available:
 *   init (string config)
 *   start (int run)
 *   stop ()
 *   shutdown (string reason)
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 */
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <stdexcept>
#include <iostream>
#include "xmlrpc_commander.hh"
#include "fhiclcpp/make_ParameterSet.h"

namespace {
  std::string exception_msg (const std::runtime_error &er) { 
    std::string msg = std::string(er.what ()).substr (2);
    if (msg[msg.size() - 1] == '\n') msg.erase(msg.size() - 1);
    return msg;
  }

  class init_: public xmlrpc_c::method { 
    public:
      init_ () { _signature = "s:s"; _help = "initialize the system"; } 
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) { 
	try { 
	  // DO STUFF
	  std::cout << "Initing with the following settings: " << paramList.getString (0) << std::endl;

//	  fhicl::make_ParameterSet (paramList.getString (0), _pset);
	  
	  *retvalP = xmlrpc_c::value_string ("ok"); 
	} catch (std::runtime_error &er) { 
	  *retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	} 
      }
  };
  
  class start_: public xmlrpc_c::method { 
    public:
      start_ () { _signature = "s:i"; _help = "start the run"; } 
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) { 
	try { 
	  // DO STUFF
	  std::cout << "starting run " << paramList.getInt (0) << std::endl;
	  
	  *retvalP = xmlrpc_c::value_string ("ok"); 
	} catch (std::runtime_error &er) { 
	  *retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	} 
      }
  };

  class stop_: public xmlrpc_c::method { 
    public:
      stop_ () { _signature = "s:n"; _help = "stop the run"; } 
      void execute (xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP) { 
	try { 
	  // DO STUFF
	  std::cout << "stopping current run" << std::endl;
	  
	  *retvalP = xmlrpc_c::value_string ("ok"); 
	} catch (std::runtime_error &er) { 
	  *retvalP = xmlrpc_c::value_string (exception_msg (er)); 
	} 
      }
  };


  class shutdown_: public xmlrpc_c::registry::shutdown {
    public:
      shutdown_ (xmlrpc_c::serverAbyss *server): _server(server) {}

      virtual void doit(const std::string& comment, void*) const {
	_server->terminate ();
      }
    private:
      xmlrpc_c::serverAbyss *_server;
  };
};


void xmlrpc_commander::do_run (int port) {
  try {
    xmlrpc_c::registry registry;

#define register_method(m) \
    xmlrpc_c::methodPtr const ptr_ ## m(new m ## _);\
    registry.addMethod ("daqtest." #m, ptr_ ## m);

    register_method(init);
    register_method(start);
    register_method(stop);

#undef register_method

    xmlrpc_c::serverAbyss server(xmlrpc_c::serverAbyss::constrOpt ().registryP (&registry).portNumber(port));

    shutdown_ shutdown(&server);
    registry.setShutdown (&shutdown);

    server.run();

  } catch (std::exception const& e) {
    std::cerr << "xml-rpc error " << e.what() << std::endl;
    return;
  }

  std::cerr << "exiting" << std::endl;
}
