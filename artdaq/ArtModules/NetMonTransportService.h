#ifndef art_Framework_IO_Root_NetMonTransportService_h
#define art_Framework_IO_Root_NetMonTransportService_h

#include "art/Framework/Services/Registry/ServiceMacros.h"

#include "artdaq/ArtModules/NetMonTransportServiceInterface.h"
#include "artdaq/DAQrate/SHandles.hh"
#include "artdaq/DAQrate/GlobalQueue.hh"

class TBufferFile;

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
    void sendMessage(uint64_t sequenceId, uint8_t messageType, TBufferFile &);
    void receiveMessage(TBufferFile *&);
private:
    size_t mpi_buffer_count_;
    uint64_t max_fragment_size_words_;
    size_t first_data_sender_rank_;
    size_t first_data_receiver_rank_;
    size_t data_sender_count_;
    size_t data_receiver_count_;

    std::unique_ptr<artdaq::SHandles> sender_ptr_;
    artdaq::RawEventQueue &incoming_events_;
    std::unique_ptr<std::vector<artdaq::Fragment> > recvd_fragments_;
};

DECLARE_ART_SERVICE_INTERFACE_IMPL(NetMonTransportService, NetMonTransportServiceInterface, LEGACY)
#endif /* art_Framework_IO_Root_NetMonTransportService_h */

// Local Variables:
// mode: c++
// End:
