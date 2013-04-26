/* DarkSide 50 DAQ program
 * This file add the xmlrpc commander as a client to the SC
 * Author: Alessandro Razeto <Alessandro.Razeto@ge.infn.it>
 */
#ifndef XMLRPC_COMMANDER_H
#define XMLRPC_COMMANDER_H

#include <mutex>
#include "ds50daq/DAQ/Commandable.hh"

class xmlrpc_commander {
  public:
    xmlrpc_commander (int port, ds50::Commandable& commandable);
    void run();

  private:
    xmlrpc_commander (const xmlrpc_commander&) = delete;
    xmlrpc_commander (xmlrpc_commander&&) = delete;

    int _port;

  public:
    ds50::Commandable& _commandable;
    std::mutex mutex_;
};

#endif
