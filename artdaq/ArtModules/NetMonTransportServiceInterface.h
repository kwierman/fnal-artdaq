#ifndef art_Framework_IO_Root_NetMonTransportServiceInterface_h
#define art_Framework_IO_Root_NetMonTransportServiceInterface_h

#include "art/Framework/Services/Registry/ServiceMacros.h"

class TMessage;

class NetMonTransportServiceInterface {
public:
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual void listen() = 0;
    virtual void sendMessage(const TMessage&) = 0;
    virtual void receiveMessage(TMessage*&) = 0;
};

DECLARE_ART_SERVICE_INTERFACE(NetMonTransportServiceInterface, LEGACY)
#endif /* art_Framework_IO_Root_NetMonTransportServiceInterface_h */

// Local Variables:
// mode: c++
// End:
