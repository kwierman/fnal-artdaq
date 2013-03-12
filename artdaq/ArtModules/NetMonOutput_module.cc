#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/Exception.h"
#include "cetlib/column_width.h"
#include "cetlib/lpad.h"
#include "cetlib/rpad.h"
#include "cpp0x/algorithm"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "unistd.h"

#include "NetMonTransportService.h"

#include "TClass.h"
#include "TMessage.h"

namespace art {
class NetMonOutput;
}

using art::NetMonOutput;
using fhicl::ParameterSet;

class art::NetMonOutput : public OutputModule {
public:
    explicit NetMonOutput(ParameterSet const&);
    ~NetMonOutput();
private:
    virtual void openFile(FileBlock const&);
    virtual void write(EventPrincipal const&);
    virtual void writeRun(RunPrincipal const&);
    virtual void writeSubRun(SubRunPrincipal const&);
private:
    //TSocket* sock_;
}; // NetMonOutput

art::NetMonOutput::
NetMonOutput(ParameterSet const& ps)
    : OutputModule(ps)
{
    //TServerSocket server(31030);
    //sock_ = server.Accept();
    //assert(sock_);
    ServiceHandle<NetMonTransportService> transport;
    transport->connect();
}

art::NetMonOutput::
~NetMonOutput()
{
    //sock_->Close();
    //delete sock_;
    //sock_ = 0;
    //sleep(10);
    ServiceHandle<NetMonTransportService> transport;
    transport->disconnect();
}

void
art::NetMonOutput::
openFile(FileBlock const& fb)
{
    (void) fb;
    fprintf(stderr, "NetMonOutput::openFile(const FileBlock&) called.\n");
    TMessage msg;
    msg.SetWriteMode();
    //
    unsigned long ps_cnt = fhicl::ParameterSetRegistry::size();
    fprintf(stderr, "openFile: parameter set count: %lu\n", ps_cnt);
    msg.WriteULong(ps_cnt);
    fprintf(stderr, "openFile: Streaming parameter sets ...\n");
    TClass* str_class = TClass::GetClass("std::string");
    assert(str_class != nullptr && "openFile: couldn't get TClass for "
           "std::string!");
    for (auto I = fhicl::ParameterSetRegistry::begin(),
            E = fhicl::ParameterSetRegistry::end(); I != E; ++I) {
        std::string pset_str = I->second.to_string();
        msg.WriteObjectAny(&pset_str, str_class);
    }
    fprintf(stderr, "openFile: Finished streaming parameter sets.\n");
    //
    ProductList productList(ProductMetaData::instance().productList());
    TClass* plc = TClass::GetClass(
        "map<art::BranchKey,art::BranchDescription>");
    assert(plc != nullptr && "openFile: couldn't get TClass for "
                             "map<art::BranchKey,art::BranchDescription>!");
    msg.WriteObjectAny(&productList, plc);
    //
    // typedef vector<BranchID::value_type> BranchIDList
    // typedef vector<BranchIDList> BranchIDLists
    // std::vector<std::vector<art::BranchID::value_type>>
    BranchIDLists *bil = &BranchIDListRegistry::instance()->data();
    TClass* bilc = TClass::GetClass("std::vector<std::vector<"
                                    "art::BranchID::value_type> >");
    assert(bilc != nullptr);
    msg.WriteObjectAny(bil, bilc);
    fprintf(stderr, "openFile: content of BranchIDLists\n");
    int max_bli = bil->size();
    fprintf(stderr, "openFile: max_bli: %d\n", max_bli);
    for (int i = 0; i < max_bli; ++i) {
       int max_prdidx = (*bil)[i].size();
       fprintf(stderr, "openFile: max_prdidx: %d\n", max_prdidx);
       for (int j = 0; j < max_prdidx; ++j) {
           fprintf(stderr, "openFile: bli: %d  prdidx: %d  bid: 0x%08lx\n",
                   i, j, static_cast<unsigned long>( (*bil)[i][j]) );
       }
    }
    //
    //
    //  Send init message.
    //
    ServiceHandle<NetMonTransportService> transport;
    fprintf(stderr, "openFile: Sending the init message ...\n");
    transport->sendMessage(msg);
    fprintf(stderr, "openFile: Init message sent.\n");
}

void
art::NetMonOutput::
write(EventPrincipal const & ep)
{
    TMessage msg;
    msg.SetWriteMode();
    std::vector<std::string> productClassNames;
    std::vector<std::string> productFriendlyClassNames;
    std::vector<std::string> productLabels;
    std::vector<std::string> productInstances;
    std::vector<std::string> productProcesses;
    //EventPrincipal::const_iterator = map<BranchID, sp<Group>>::iterator
    for (EventPrincipal::const_iterator I = ep.begin(), E = ep.end();
            I != E; ++I) {
        if (I->second->productUnavailable()) {
            continue;
        }
        const BranchDescription& bd(I->second->productDescription());
        BranchKey bk(bd);
        fprintf(stderr, "Saw product class: '%s' modlbl: '%s' instnm: '%s' procnm: '%s'\n",
            bd.friendlyClassName().c_str(),
            bd.moduleLabel().c_str(),
            bd.productInstanceName().c_str(),
            bd.processName().c_str()
        );
        productClassNames.push_back(bd.producedClassName());
        productFriendlyClassNames.push_back(bd.friendlyClassName());
        productLabels.push_back(bd.moduleLabel());
        productInstances.push_back(bd.productInstanceName());
        productProcesses.push_back(bd.processName());
    }
    TClass* vs = TClass::GetClass("vector<string>");
    msg.WriteObjectAny(&productClassNames, vs);
    msg.WriteObjectAny(&productFriendlyClassNames, vs);
    msg.WriteObjectAny(&productLabels, vs);
    msg.WriteObjectAny(&productInstances, vs);
    msg.WriteObjectAny(&productProcesses, vs);
    for (EventPrincipal::const_iterator I = ep.begin(), E = ep.end();
            I != E; ++I) {
        if (I->second->productUnavailable()) {
            continue;
        }
        const BranchDescription& bd = I->second->productDescription();
        const std::string& name = bd.producedClassName();
        if (name == "art::TriggerResults") {
            continue;
        }
        fprintf(stderr, "Streaming product of class: '%s' instnm: '%s'\n",
             name.c_str(),
             bd.productInstanceName().c_str()
        );
        OutputHandle oh = ep.getForOutput(bd.branchID(), true);
        //const EDProduct* prd = I->second->getIt();
        const EDProduct* prd = oh.wrapper();
        msg.WriteObjectAny(prd, TClass::GetClass(bd.wrappedName().c_str()));
    }
    ServiceHandle<NetMonTransportService> transport;
    fprintf(stderr, "Sending a message ...\n");
    transport->sendMessage(msg);
    //sock_->Send(msg);
    fprintf(stderr, "Message sent.\n");
}

void
art::NetMonOutput::
writeRun(RunPrincipal const&)
{
}

void
art::NetMonOutput::writeSubRun(SubRunPrincipal const&)
{
}

DEFINE_ART_MODULE(art::NetMonOutput)

