/* High Precision Timing Facility
 *
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 *
 * $Id: commander.hh,v 1.1 2012-01-06 15:22:03 razeto Exp $
 */
#ifndef XMLRPC_COMMANDER_H
#define XMLRPC_COMMANDER_H

class xmlrpc_commander {
  public:
    static void run (int port) { xmlrpc_commander().do_run (port); }

  private:
    xmlrpc_commander () = default;
    xmlrpc_commander (const xmlrpc_commander&) = delete;
    xmlrpc_commander (xmlrpc_commander&&) = delete;

    void do_run (int port);
};


#endif
/*
 * $Log: commander.hh,v $
 * Revision 1.1  2012-01-06 15:22:03  razeto
 * Added a working basilar commander
 *
 */
