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
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/Exception.h"
#include "cetlib/column_width.h"
#include "cetlib/lpad.h"
#include "cetlib/rpad.h"
#include "cpp0x/algorithm"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include "artdaq/ArtModules/NetMonTransportService.h"
#include "artdaq/DAQdata/NetMonHeader.hh"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "unistd.h"

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
}; // NetMonOutput

art::NetMonOutput::
NetMonOutput(ParameterSet const& ps)
    : OutputModule(ps)
{
    ServiceHandle<NetMonTransportService> transport;
    transport->connect();
}

art::NetMonOutput::
~NetMonOutput()
{
    ServiceHandle<NetMonTransportService> transport;
    transport->disconnect();
}

void
art::NetMonOutput::
openFile(FileBlock const& fb)
{
    (void) fb;
    fprintf(stderr, "NetMonOutput::openFile(const FileBlock&) called.\n");
    //
    //  Construct and send the init message.
    //
    TMessage msg;
    msg.SetWriteMode();
    //
    //  Stream the message type code.
    //
    fprintf(stderr, "openFile: streaming message type code ...\n");
    msg.WriteULong(1);
    fprintf(stderr, "openFile: finished streaming message type code.\n");
    //
    //  Stream the ParameterSetRegistry.
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
    //  Stream the MasterProductRegistry.
    //
    fprintf(stderr, "openFile: Streaming MasterProductRegistry ...\n");
    ProductList productList(ProductMetaData::instance().productList());
    TClass* plc = TClass::GetClass(
        "map<art::BranchKey,art::BranchDescription>");
    assert(plc != nullptr && "openFile: couldn't get TClass for "
                             "map<art::BranchKey,art::BranchDescription>!");
    msg.WriteObjectAny(&productList, plc);
    fprintf(stderr, "openFile: finished streaming MasterProductRegistry.\n");
    //
    //  Stream the BranchIDListRegistry.
    //
    fprintf(stderr, "openFile: Streaming BranchIDListRegistry ...\n");
    // typedef vector<BranchID::value_type> BranchIDList
    // typedef vector<BranchIDList> BranchIDLists
    // std::vector<std::vector<art::BranchID::value_type>>
    BranchIDLists *bil = &BranchIDListRegistry::instance()->data();
    TClass* bilc = TClass::GetClass("std::vector<std::vector<"
                                    "art::BranchID::value_type> >");
    assert(bilc != nullptr);
    msg.WriteObjectAny(bil, bilc);
    fprintf(stderr, "openFile: finished streaming BranchIDListRegistry.\n");
    {
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
    }
    //
    //  Stream the ProcessHistoryRegistry.
    //
    {
        fprintf(stderr, "openFile: dumping ProcessHistoryRegistry ...\n");
        ProcessHistoryMap const& phr = ProcessHistoryRegistry::get();
        fprintf(stderr, "openFile: phr: size: %lu\n",
                (unsigned long) phr.size());
        for (auto I = phr.begin(), E = phr.end(); I != E; ++I) {
            std::ostringstream OS;
            I->first.print(OS);
            fprintf(stderr, "openFile: phr: id: '%s'\n", OS.str().c_str());
        }
    }
    fprintf(stderr, "openFile: Streaming ProcessHistoryRegistry ...\n");
    //typedef std::map<const ProcessHistoryID,ProcessHistory> ProcessHistoryMap;
    const ProcessHistoryMap& phm = ProcessHistoryRegistry::get();
    fprintf(stderr, "openFile: phm: size: %lu\n", (unsigned long) phm.size());
    //TClass* phm_class = TClass::GetClass("std::map<const art::ProcessHistoryID,art::ProcessHistory>");
    TClass* phm_class = TClass::GetClass("std::map<const art::Hash<2>,art::ProcessHistory>");
    assert(phm_class != nullptr);
    msg.WriteObjectAny(&phm, phm_class);
    fprintf(stderr, "openFile: finished streaming ProcessHistoryRegistry.\n");
    //
    //  Stream the ParentageRegistry.
    //
    fprintf(stderr, "openFile: Streaming ParentageRegistry ...\n");
    //typedef std::map<const ParentageID,Parentage> ParentageMap
    const ParentageMap& parentageMap = ParentageRegistry::get();
    //TClass* parentageMapClass = TClass::GetClass("std::map<const art::ParentageID,art::Parentage>");
    TClass* parentageMapClass = TClass::GetClass("std::map<const art::Hash<5>,art::Parentage>");
    assert(parentageMapClass != nullptr);
    msg.WriteObjectAny(&parentageMap, parentageMapClass);
    fprintf(stderr, "openFile: finished streaming ParentageRegistry.\n");
    //
    //
    //  Send init message.
    //
    ServiceHandle<NetMonTransportService> transport;
    fprintf(stderr, "openFile: Sending the init message ...\n");
    transport->sendMessage(0, artdaq::NetMonHeader::InitDataFragmentType, msg);
    fprintf(stderr, "openFile: Init message sent.\n");
}

void
art::NetMonOutput::
write(EventPrincipal const & ep)
{
    TMessage msg;
    msg.SetWriteMode();
    //fprintf(stderr, "write: streaming message type code ...\n");
    msg.WriteULong(4);
    //fprintf(stderr, "write: finished streaming message type code.\n");
    //fprintf(stderr, "write: streaming EventAuxiliary ...\n");
    static TClass* aux_class = TClass::GetClass("art::EventAuxiliary");
    assert(aux_class != nullptr && "write: Could not get TClass for "
           "art::EventAuxiliary!");
    msg.WriteObjectAny(&ep.aux(), aux_class);
    //fprintf(stderr, "write: streamed EventAuxiliary.\n");
    //fprintf(stderr, "write: streaming History ...\n");
    static TClass* history_class = TClass::GetClass("art::History");
    assert(history_class != nullptr && "write: Could not get TClass for "
           "art::History!");
    msg.WriteObjectAny(&ep.history(), history_class);
    //fprintf(stderr, "write: streamed History.\n");
    //std::vector<std::string> productClassNames;
    //std::vector<std::string> productFriendlyClassNames;
    //std::vector<std::string> productLabels;
    //std::vector<std::string> productInstances;
    //std::vector<std::string> productProcesses;
    unsigned long prd_cnt = 0;
    //EventPrincipal::const_iterator = map<BranchID, sp<Group>>::iterator
    for (EventPrincipal::const_iterator I = ep.begin(), E = ep.end();
            I != E; ++I) {
        if (I->second->productUnavailable()) {
            continue;
        }
        const BranchDescription& bd(I->second->productDescription());
        const std::string& name = bd.producedClassName();
        if (name == "art::TriggerResults") {
            continue;
        }
        BranchKey bk(bd);
        //fprintf(stderr, "write: aw product class: '%s' modlbl: '%s' "
        //    "instnm: '%s' procnm: '%s'\n",
        //    bd.friendlyClassName().c_str(),
        //    bd.moduleLabel().c_str(),
        //   bd.productInstanceName().c_str(),
        //    bd.processName().c_str()
        //);
        //productClassNames.push_back(bd.producedClassName());
        //productFriendlyClassNames.push_back(bd.friendlyClassName());
        //productLabels.push_back(bd.moduleLabel());
        //productInstances.push_back(bd.productInstanceName());
        //productProcesses.push_back(bd.processName());
        ++prd_cnt;
    }
    //fprintf(stderr, "write: streaming product count: %lu\n", prd_cnt);
    msg.WriteULong(prd_cnt);
    //fprintf(stderr, "write: finished streaming product count.\n");
    //static TClass* vs = TClass::GetClass("vector<string>");
    //msg.WriteObjectAny(&productClassNames, vs);
    //msg.WriteObjectAny(&productFriendlyClassNames, vs);
    //msg.WriteObjectAny(&productLabels, vs);
    //msg.WriteObjectAny(&productInstances, vs);
    //msg.WriteObjectAny(&productProcesses, vs);
    static TClass* branch_key_class = TClass::GetClass("art::BranchKey");
    assert(branch_key_class != nullptr && "write: Could not get TClass for "
           "art::BranchKey!");
    static TClass* prdprov_class = TClass::GetClass("art::ProductProvenance");
    assert(prdprov_class != nullptr && "write: Could not get TClass for "
           "art::ProductProvenance!");
    std::vector<BranchKey> bkv;
    //std::map<art::BranchID, std::shared_ptr<art::Group>>::const_iterator
    for (EventPrincipal::const_iterator I = ep.begin(), E = ep.end();
            I != E; ++I) {
        if (I->second->productUnavailable()) {
            continue;
        }
        const BranchDescription& bd(I->second->productDescription());
        const std::string& name = bd.producedClassName();
        if (name == "art::TriggerResults") {
            continue;
        }
        //fprintf(stderr, "write: saw product class: '%s' modlbl: '%s' "
        //    "instnm: '%s' procnm: '%s'\n",
        //    bd.friendlyClassName().c_str(),
        //    bd.moduleLabel().c_str(),
        //    bd.productInstanceName().c_str(),
        //    bd.processName().c_str()
        //);
        bkv.push_back(BranchKey(bd));
        //fprintf(stderr, "write: streaming branch key of class: '%s' "
        //     "instnm: '%s'\n", name.c_str(), bd.productInstanceName().c_str());
        //fprintf(stderr, "write: dumping branch key: class: '%s' modlbl: '%s' "
        //    "instnm: '%s' procnm: '%s'\n",
        //    bkv.back().friendlyClassName_.c_str(),
        //    bkv.back().moduleLabel_.c_str(),
        //    bkv.back().productInstanceName_.c_str(),
        //    bkv.back().processName_.c_str()
        //);
        msg.WriteObjectAny(&bkv.back(), branch_key_class);
        //fprintf(stderr, "write: streaming product of class: '%s' "
        //     "instnm: '%s'\n", name.c_str(), bd.productInstanceName().c_str());
        OutputHandle oh = ep.getForOutput(bd.branchID(), true);
        //const EDProduct* prd = I->second->getIt();
        const EDProduct* prd = oh.wrapper();
        msg.WriteObjectAny(prd, TClass::GetClass(bd.wrappedName().c_str()));
        //fprintf(stderr, "write: streaming product provenance of class: '%s' "
        //     "instnm: '%s'\n", name.c_str(), bd.productInstanceName().c_str());
        const ProductProvenance* prdprov =
            I->second->productProvenancePtr().get();
        msg.WriteObjectAny(prdprov, prdprov_class);
    }

    ServiceHandle<NetMonTransportService> transport;
    transport->sendMessage(static_cast<uint64_t>(ep.id().event()), 
			   artdaq::NetMonHeader::EventDataFragmentType, 
			   msg);
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

