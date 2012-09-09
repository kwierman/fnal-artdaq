/* DarkSide 50 DAQ program
 * This file add the xmlrpc commander as a client to the SC
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 */
#ifndef XMLRPC_COMMANDER_H
#define XMLRPC_COMMANDER_H

#include "fhiclcpp/ParameterSet.h"

class xmlrpc_commander {
  public:
    static void run (int port) { xmlrpc_commander().do_run (port); }

  private:
    xmlrpc_commander () = default;
    xmlrpc_commander (const xmlrpc_commander&) = delete;
    xmlrpc_commander (xmlrpc_commander&&) = delete;

    void do_run (int port);

    fhicl::ParameterSet _pset;

    enum state {
      idle,
      inited,
      running,
      paused
    };
};


#endif
