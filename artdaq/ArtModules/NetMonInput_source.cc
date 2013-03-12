#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/PrincipalMaker.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/Framework/IO/Sources/SourceTraits.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include "NetMonTransportService.h"

#include "TClass.h"
#include "TMessage.h"

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace art {
class NetMonInputDetail;
}

class art::NetMonInputDetail {
public:
    NetMonInputDetail(const NetMonInputDetail&) = delete;
    NetMonInputDetail& operator=(const NetMonInputDetail&) = delete;

    ~NetMonInputDetail();

    NetMonInputDetail(const fhicl::ParameterSet&, art::ProductRegistryHelper&,
                      const art::PrincipalMaker&, art::MasterProductRegistry&);

    void closeCurrentFile();

    void readFile(const std::string&, art::FileBlock*&,
                  art::MasterProductRegistry&);

    bool readNext(art::RunPrincipal* const& inR,
                  art::SubRunPrincipal* const& inSR, art::RunPrincipal*& outR,
                  art::SubRunPrincipal*& outSR, art::EventPrincipal*& outE);

private:
    bool readFileCalled_;
    const art::PrincipalMaker& pm_;
    art::EventNumber_t ev_num_;
    const int max_events_;
    //TSocket* sock_;
};

art::NetMonInputDetail::
NetMonInputDetail(const fhicl::ParameterSet& ps,
                  art::ProductRegistryHelper& helper,
                  const art::PrincipalMaker& pm,
                  art::MasterProductRegistry& mpr)
    : readFileCalled_(false),
      pm_(pm),
      ev_num_(0),
      max_events_(ps.get<int>("maxEvents", 1))
{
    (void) helper;
    //helper.reconstitutes<NetMonProd1, art::InEvent>("nm1");
    //helper.reconstitutes<NetMonProd2, art::InEvent>("nm1");
    //helper.reconstitutes<NetMonProd2, art::InEvent>("nm2",
    //                                                "InstNetMonProducer2");
    //sock_ = new TSocket("localhost", 31030);
    fprintf(stderr, "readFile: Starting server ...\n");
    ServiceHandle<NetMonTransportService> transport;
    transport->listen();
    fprintf(stderr, "readFile: Connect request received.\n");
    TMessage* msg_ptr = 0;
    transport->receiveMessage(msg_ptr);
    fprintf(stderr, "readFile: receiveMessage returned.  ptr: 0x%lx\n",
            (unsigned long) msg_ptr);
    assert(msg_ptr != nullptr);
    std::unique_ptr<TMessage> msg(msg_ptr);
    msg_ptr = 0;
    void* p = 0;
    //
    //
    //
    unsigned long ps_cnt = 0;
    msg->ReadULong(ps_cnt);
    fprintf(stderr, "readFile: parameter set count: %lu\n", ps_cnt);
    fprintf(stderr, "readFile: reading parameter sets ...\n");
    TClass* str_class = TClass::GetClass("std::string");
    assert(str_class != nullptr && "readFile: couldn't get TClass for "
           "std::string!");
    for (unsigned long I = 0; I < ps_cnt; ++I) {
        p = msg->ReadObjectAny(str_class);
        std::string* pset_str = reinterpret_cast<std::string*>(p);
        p = 0;
        fhicl::ParameterSet pset;
        fhicl::make_ParameterSet(*pset_str, pset);
        // Force id calculation.
        pset.id();
        fhicl::ParameterSetRegistry::put(pset);
    }
    fprintf(stderr, "readFile: finished reading parameter sets.\n");
    //
    //
    //
    TClass* plc = TClass::GetClass("map<art::BranchKey,"
                                   "art::BranchDescription>");
    assert(plc != nullptr && "readFile: couldn't get TClass for "
           "map<art::BranchKey,art::BranchDescription>!");
    p = msg->ReadObjectAny(plc);
    art::ProductList* productList = reinterpret_cast<art::ProductList*>(p);
    p = 0;
    fprintf(stderr, "readFile: got product list\n");
    //
    //
    //
    for (art::ProductList::iterator I = productList->begin(),
            E = productList->end(); I != E; ++I) {
        I->second.setPresent(false);
    }
    for (art::ProductList::const_iterator I = productList->begin(),
            E = productList->end(); I != E; ++I) {
        std::cerr << I->first << '\n';
        std::cerr << "present: " << I->second.present() << '\n';
        std::cerr << "produced: " << I->second.produced() << '\n';
    }
    //
    //
    //
    std::string empty;
    fprintf(stderr, "readFile: merging product list ...\n");
    mpr.merge(*productList, empty, art::BranchDescription::Strict);
    fprintf(stderr, "readFile: product list merged.\n");
    delete productList;
    //
    //
    //
    art::ProductList& pl = const_cast<art::ProductList&>(mpr.productList());
    for (art::ProductList::iterator I = pl.begin(), E = pl.end(); I != E; ++I) {
        I->second.setPresent(true);
        //I->second.setProduced(true);
    }
    for (art::ProductList::const_iterator I = mpr.productList().begin(),
            E = mpr.productList().end(); I != E; ++I) {
        std::cerr << I->first << '\n';
        std::cerr << "present: " << I->second.present() << '\n';
        std::cerr << "produced: " << I->second.produced() << '\n';
    }
    //
    //
    //
    {
        fprintf(stderr, "readFile: before BranchIDLists\n");
        BranchIDLists *bil = &BranchIDListRegistry::instance()->data();
        int max_bli = bil->size();
        fprintf(stderr, "readFile: max_bli: %d\n", max_bli);
        for (int i = 0; i < max_bli; ++i) {
           int max_prdidx = (*bil)[i].size();
           fprintf(stderr, "readFile: max_prdidx: %d\n", max_prdidx);
           for (int j = 0; j < max_prdidx; ++j) {
               fprintf(stderr, "readFile: bli: %d  prdidx: %d  bid: 0x%08lx\n",
                       i, j, static_cast<unsigned long>( (*bil)[i][j]) );
           }
        }
    }
    //
    //
    //
    TClass* bilc = TClass::GetClass("std::vector<std::vector<"
                                    "art::BranchID::value_type> >");
    assert(bilc != nullptr && "readFile: couldn't get TClass for "
           "std::vector<std::vector<art::BranchID::value_type> >!");
    p = msg->ReadObjectAny(bilc);
    BranchIDLists* bil = reinterpret_cast<art::BranchIDLists*>(p);
    p = 0;
    fprintf(stderr, "readFile: got BranchIDLists\n");
    {
        int max_bli = bil->size();
        fprintf(stderr, "readFile: max_bli: %d\n", max_bli);
        for (int i = 0; i < max_bli; ++i) {
           int max_prdidx = (*bil)[i].size();
           fprintf(stderr, "readFile: max_prdidx: %d\n", max_prdidx);
           for (int j = 0; j < max_prdidx; ++j) {
               fprintf(stderr, "readFile: bli: %d  prdidx: %d  bid: 0x%08lx\n",
                       i, j, static_cast<unsigned long>( (*bil)[i][j]) );
           }
        }
    }
    BranchIDListHelper::updateFromInput(*bil, "NetMonInput");
    {
        fprintf(stderr, "readFile: after BranchIDListHelper::updateFromInput(BranchIDLists)\n");
        BranchIDLists *bil = &BranchIDListRegistry::instance()->data();
        int max_bli = bil->size();
        fprintf(stderr, "readFile: max_bli: %d\n", max_bli);
        for (int i = 0; i < max_bli; ++i) {
           int max_prdidx = (*bil)[i].size();
           fprintf(stderr, "readFile: max_prdidx: %d\n", max_prdidx);
           for (int j = 0; j < max_prdidx; ++j) {
               fprintf(stderr, "readFile: bli: %d  prdidx: %d  bid: 0x%08lx\n",
                       i, j, static_cast<unsigned long>( (*bil)[i][j]) );
           }
        }
    }
}

art::NetMonInputDetail::
~NetMonInputDetail()
{
    //sock_->Close();
    //delete sock_;
    //sock_ = 0;
    ServiceHandle<NetMonTransportService> transport;
    transport->disconnect();
}

void
art::NetMonInputDetail::
closeCurrentFile()
{
    //ServiceHandle<NetMonTransportService> transport;
    //transport->disconnect();
}

void
art::NetMonInputDetail::
readFile(const std::string&, art::FileBlock*& fb,
         art::MasterProductRegistry&)
{
    assert(!readFileCalled_);
    //assert(name.empty());
    readFileCalled_ = true;
    fb = new art::FileBlock(art::FileFormatVersion(1, "NetMonInput2013"),
                            "nothing");
}

bool
art::NetMonInputDetail::
readNext(art::RunPrincipal* const& inR,
         art::SubRunPrincipal* const& inSR,
         art::RunPrincipal*& outR,
         art::SubRunPrincipal*& outSR,
         art::EventPrincipal*& outE)
{
    art::Timestamp runstart;
    ++ev_num_;
    if (ev_num_ > static_cast<size_t>(max_events_)) {
        return false;
    }
    if (inR == nullptr) {
        outR = pm_.makeRunPrincipal(1, runstart);
    }
    if (inSR == nullptr) {
        art::SubRunID srid(outR ? outR->run() : inR->run(), 0);
        outSR = pm_.makeSubRunPrincipal(srid.run(), srid.subRun(), runstart);
    }
    outE = pm_.makeEventPrincipal(outR ? outR->run() : inR->run(),
                                  outSR ? outSR->subRun() : inSR->subRun(),
                                  ev_num_, runstart);
    const ProductList& productList = ProductMetaData::instance().productList();
    fprintf(stderr, "Calling receiveMessage ...\n");
    ServiceHandle<NetMonTransportService> transport;
    TMessage* msg_ptr = 0;
    transport->receiveMessage(msg_ptr);
    fprintf(stderr, "receiveMessage returned.  ptr: 0x%lx\n", (unsigned long) msg_ptr);
    assert(msg_ptr != nullptr);
    //int nBytes = sock_->Recv(msg_ptr);
    //assert(nBytes > 0);
    std::unique_ptr<TMessage> msg(msg_ptr);
    msg_ptr = 0;
    TClass* vs = TClass::GetClass("vector<string>");
    void* p = 0;
    p = msg->ReadObjectAny(vs);
    std::unique_ptr<std::vector<std::string>> productClassNames(
            reinterpret_cast<std::vector<std::string>*>(p));
    p = msg->ReadObjectAny(vs);
    std::unique_ptr<std::vector<std::string>> productFriendlyClassNames(
            reinterpret_cast<std::vector<std::string>*>(p));
    p = msg->ReadObjectAny(vs);
    std::unique_ptr<std::vector<std::string>> productLabels(
            reinterpret_cast<std::vector<std::string>*>(p));
    p = msg->ReadObjectAny(vs);
    std::unique_ptr<std::vector<std::string>> productInstances(
            reinterpret_cast<std::vector<std::string>*>(p));
    p = msg->ReadObjectAny(vs);
    std::unique_ptr<std::vector<std::string>> productProcesses(
            reinterpret_cast<std::vector<std::string>*>(p));
    for (std::vector<std::string>::size_type I = 0,
            E = productClassNames->size(); I != E; ++I) {
        std::string name(productClassNames->at(I));
        if (name == "art::TriggerResults") {
            continue;
        }
        fprintf(stderr, "Reading product of class %s\n", name.c_str());
        std::string friendly_class_name(productFriendlyClassNames->at(I));
        std::string module_label(productLabels->at(I));
        std::string instance_name(productInstances->at(I));
        std::string process_name(productProcesses->at(I));
        BranchKey bk(friendly_class_name, module_label, instance_name,
                     process_name);
        ProductList::const_iterator iter = productList.find(bk);
        if (iter == productList.end()) {
            throw art::Exception(art::errors::InsertFailure)
                    << "No product is registered for\n"
                    << "  process name:                '"
                    << bk.processName_ << "'\n"
                    << "  module label:                '"
                    << bk.moduleLabel_ << "'\n"
                    << "  product friendly class name: '"
                    << bk.friendlyClassName_ << "'\n"
                    << "  product instance name:       '"
                    << bk.productInstanceName_ << "'\n";
        }
        if (iter->second.branchType() != art::InEvent) {
            throw art::Exception(art::errors::InsertFailure, "Not Registered")
                    << "The product for ("
                    << bk.friendlyClassName_ << ","
                    << bk.moduleLabel_ << ","
                    << bk.productInstanceName_ << ","
                    << bk.processName_
                    << ")\n"
                    << "is registered for a(n) " << iter->second.branchType()
                    << " instead of for a(n) " << art::InEvent
                    << ".\n";
        }
        // Note: This must be a reference to the unique copy in
        //       the master product registry!
        const BranchDescription& bd = iter->second;
        p = msg->ReadObjectAny(TClass::GetClass(bd.wrappedName().c_str()));
        std::unique_ptr<EDProduct> prd(reinterpret_cast<EDProduct*>(p));
        p = 0;
        std::unique_ptr<const ProductProvenance> prdprov(
            new ProductProvenance(bd.branchID(), productstatus::present()));
        //void put(std::unique_ptr<EDProduct>&&, BranchDescription const&,
        //         std::unique_ptr<ProductProvenance const>&&);
        fprintf(stderr, "Inserting product class: '%s' modlbl: '%s' instnm: '%s' procnm: '%s'\n",
                bd.friendlyClassName().c_str(),
                bd.moduleLabel().c_str(),
                bd.productInstanceName().c_str(),
                bd.processName().c_str()
               );
        outE->put(std::move(prd), bd, std::move(prdprov));
    }
    return true;
}

namespace art {

// Trait definition (must precede source typedef).
template <>
struct Source_generator<art::NetMonInputDetail> {
    static constexpr bool value = true;
};

// Source declaration.
typedef art::Source<NetMonInputDetail> NetMonInput;

} // namespace art

DEFINE_ART_INPUT_SOURCE(art::NetMonInput)

