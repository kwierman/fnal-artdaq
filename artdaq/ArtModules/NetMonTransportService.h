#ifndef art_Framework_IO_Root_NetMonTransportService_h
#define art_Framework_IO_Root_NetMonTransportService_h

#include "NetMonTransportServiceInterface.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

#include "TServerSocket.h"

class TMessage;
class TSocket;

namespace art {
class ActivityRegistry;
}

namespace fhicl {
class ParameterSet;
}

// ----------------------------------------------------------------------

class NetMonTransportService : public NetMonTransportServiceInterface {
public:
    ~NetMonTransportService();
    NetMonTransportService(fhicl::ParameterSet const&, art::ActivityRegistry&);
    void connect();
    void disconnect();
    void listen();
    void sendMessage(TMessage const&);
    void receiveMessage(TMessage*&);
private:
    TServerSocket* server_sock_;
    TSocket* sock_;
};

DECLARE_ART_SERVICE_INTERFACE_IMPL(NetMonTransportService, NetMonTransportServiceInterface, LEGACY)
#endif /* art_Framework_IO_Root_NetMonTransportService_h */

// Local Variables:
// mode: c++
// End:
