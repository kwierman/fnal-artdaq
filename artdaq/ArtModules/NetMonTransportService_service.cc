#include "NetMonTransportService.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "TClass.h"
#include "TServerSocket.h"
#include "TMessage.h"

using namespace cet;
using namespace fhicl;
using namespace std;

static ParameterSet empty_pset;

NetMonTransportService::
~NetMonTransportService()
{
    disconnect();
}

NetMonTransportService::
NetMonTransportService(ParameterSet const& pset, art::ActivityRegistry&)
    : NetMonTransportServiceInterface(), server_sock_(0), sock_(0)
{
    mf::LogVerbatim("DEBUG") <<
        "-----> Begin NetMonTransportService::"
        "NetMonTransportService(ParameterSet const & pset, "
        "art::ActivityRegistry&)";
    //ParameterSet services = pset.get<ParameterSet>("services", empty_pset);
    string val = pset.to_indented_string();
    mf::LogVerbatim("DEBUG") << "Contents of parameter set:";
    mf::LogVerbatim("DEBUG") << "";
    mf::LogVerbatim("DEBUG") << val;
    vector<string> keys = pset.get_pset_keys();
    for (vector<string>::iterator I = keys.begin(), E = keys.end();
            I != E; ++I) {
        mf::LogVerbatim("DEBUG") << "key: " << *I;
    }
    mf::LogVerbatim("DEBUG") << "this: 0x" << std::hex << this << std::dec;
    mf::LogVerbatim("DEBUG") <<
        "-----> End   NetMonTransportService::"
        "NetMonTransportService(ParameterSet const & pset, "
        "art::ActivityRegistry&)";
}

void
NetMonTransportService::
connect()
{
    assert(sock_ == nullptr);
    sock_ = new TSocket("localhost", 31030);
    assert(sock_ != nullptr);
}

void
NetMonTransportService::
disconnect()
{
    if (sock_ != nullptr) {
        sock_->Close();
    }
    delete sock_;
    sock_ = 0;
    if (server_sock_ != nullptr) {
        server_sock_->Close();
    }
    delete server_sock_;
    server_sock_ = 0;
}

void
NetMonTransportService::
listen()
{
    assert(sock_ == nullptr);
    assert(server_sock_ == nullptr);
    server_sock_ = new TServerSocket(31030);
    assert(server_sock_ != nullptr);
    sock_ = server_sock_->Accept();
    assert(sock_ != nullptr);
}

void
NetMonTransportService::
sendMessage(TMessage const& msg)
{
    assert(sock_ != nullptr);
    sock_->Send(msg);
}

void
NetMonTransportService::
receiveMessage(TMessage*& msg)
{
    assert(sock_ != nullptr);
    assert(msg == nullptr);
    int nBytes = sock_->Recv(msg);
    assert(nBytes > 0);
    mf::LogVerbatim("DEBUG") << "transport received a message: " << nBytes << " bytes.";
}

DEFINE_ART_SERVICE_INTERFACE_IMPL(NetMonTransportService,
                                  NetMonTransportServiceInterface)

