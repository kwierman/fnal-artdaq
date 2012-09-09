/* High Precision Timing Facility
 *
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 *
 * $Id: commander.cc,v 1.11 2012-04-01 19:55:13 razeto Exp $
 */
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <stdexcept>
#include <iostream>
#include "xmlrpc_commander.hh"

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
/*
 * $Log: commander.cc,v $
 * Revision 1.11  2012-04-01 19:55:13  razeto
 * Added pwd to init/start/stop/abort/reload/write_file/shutdown
 *
 * Revision 1.10  2012-03-21 15:24:26  razeto
 * Skip hidden files
 *
 * Revision 1.9  2012-03-12 18:49:08  razeto
 * Added logfile generation + commander interface to log file name
 *
 * Revision 1.8  2012-03-12 15:36:09  razeto
 * Added process status to commander
 *
 * Revision 1.7  2012-01-22 09:43:31  razeto
 * Fixed hptf + now read can read partial files
 *
 * Revision 1.6  2012-01-12 10:23:59  razeto
 * Added file utils (read, write, list)
 *
 * Revision 1.5  2012-01-12 09:43:45  razeto
 * Added abort and reload. Using macros for repetitive tasks
 *
 * Revision 1.4  2012-01-10 09:34:57  razeto
 * added level xmlrpc method
 *
 * Revision 1.3  2012-01-06 15:22:03  razeto
 * Added a working basilar commander
 *
 */
