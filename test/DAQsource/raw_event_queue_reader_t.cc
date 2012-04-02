#include "artdaq/DAQsource/detail/RawEventQueueReader.hh"
#include "artdaq/DAQrate/GlobalQueue.hh"

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/PrincipalMaker.h"
#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"

#include "art/Utilities/Exception.h"

#include <iostream>
#include <memory>
#include <string>

using std::string;
using std::cerr;
using std::endl;

using art::EventID;
using art::EventPrincipal;
using art::FileBlock;
using art::FileFormatVersion;
using art::PrincipalMaker;
using art::PrincipalMaker;
using art::ProcessConfiguration;
using art::ProductRegistryHelper;
using art::RunID;
using art::RunPrincipal;
using art::SubRunID;
using art::SubRunPrincipal;
using art::Timestamp;
using artdaq::RawEvent;
using artdaq::RawEvent_ptr;
using artdaq::detail::RawEventQueueReader;
using fhicl::ParameterSet;




class MPRGlobalTestFixture {
public:
  MPRGlobalTestFixture();

  typedef std::map<std::string, art::BranchKey> BKmap_t;

  BKmap_t branchKeys_;
  std::map<std::string, art::ProcessConfiguration*> processConfigurations_;

private:
  art::ProcessConfiguration*
  fake_single_module_process(std::string const& tag,
                             std::string const& processName,
                             fhicl::ParameterSet const& moduleParams,
                             std::string const& release = art::getReleaseVersion(),
                             std::string const& pass = art::getPassID() );

  std::auto_ptr<art::BranchDescription>
  fake_single_process_branch(std::string const& tag,
                             std::string const& processName,
                             std::string const& productInstanceName = std::string() );

  art::MasterProductRegistry  productRegistry_;
  art::RootDictionaryManager rdm_;
};


MPRGlobalTestFixture::MPRGlobalTestFixture()
  :
  branchKeys_(),
  processConfigurations_(),
  productRegistry_(),
  rdm_()
{
  // We can only insert products registered in the MasterProductRegistry.
  productRegistry_.addProduct(fake_single_process_branch("hlt",  "HLT"));
  productRegistry_.addProduct(fake_single_process_branch("prod", "PROD"));
  productRegistry_.addProduct(fake_single_process_branch("test", "TEST"));
  productRegistry_.addProduct(fake_single_process_branch("user", "USER"));
  productRegistry_.addProduct(fake_single_process_branch("rick", "USER2", "rick"));
  productRegistry_.setFrozen();
  art::BranchIDListHelper::updateRegistries(productRegistry_);
  art::ProductMetaData::create_instance(productRegistry_);
}

art::ProcessConfiguration*
MPRGlobalTestFixture::
fake_single_module_process(std::string const& tag,
                           std::string const& processName,
                           fhicl::ParameterSet const& moduleParams,
                           std::string const& release,
                           std::string const& pass)
{
  fhicl::ParameterSet processParams;
  processParams.put(processName, moduleParams);
  processParams.put<std::string>("process_name",
                                          processName);

  art::ProcessConfiguration* result =
    new art::ProcessConfiguration(processName, processParams.id(), release, pass);
  processConfigurations_[tag] = result;
  return result;
}

std::auto_ptr<art::BranchDescription>
MPRGlobalTestFixture::
fake_single_process_branch(std::string const& tag,
                           std::string const& processName,
                           std::string const& productInstanceName)
{
  art::ModuleDescription mod;
  std::string moduleLabel = processName + "dummyMod";
  std::string moduleClass("DummyModule");
  art::TypeID dummyType(typeid(int));
  fhicl::ParameterSet modParams;
  modParams.put<std::string>("module_type", moduleClass);
  modParams.put<std::string>("module_label", moduleLabel);
  mod.parameterSetID_ = modParams.id();
  mod.moduleName_ = moduleClass;
  mod.moduleLabel_ = moduleLabel;
  art::ProcessConfiguration* process =
    fake_single_module_process(tag, processName, modParams);
  mod.processConfiguration_ = *process;

  art::BranchDescription* result =
    new art::BranchDescription(art::TypeLabel(art::InEvent,
                                              dummyType,
                                              productInstanceName),
                               mod);
  branchKeys_.insert(std::make_pair(tag, art::BranchKey(*result)));
  return std::auto_ptr<art::BranchDescription>(result);
}

struct EventPrincipalTestFixture {
  typedef MPRGlobalTestFixture::BKmap_t BKmap_t;

  EventPrincipalTestFixture();

  MPRGlobalTestFixture &gf();

  std::auto_ptr<art::EventPrincipal> pEvent_;
};

EventPrincipalTestFixture::EventPrincipalTestFixture()
  :
  pEvent_()
{
  (void) gf(); // Bootstrap MasterProductRegistry creation first time out.

  art::EventID eventID(101, 87, 20);

  // Making a functional EventPrincipal is not trivial, so we do it
  // all here.

  // Put products we'll look for into the EventPrincipal.
  typedef int PRODUCT_TYPE;
  typedef art::Wrapper<PRODUCT_TYPE> WDP;
  std::auto_ptr<art::EDProduct>  product(new WDP(std::auto_ptr<PRODUCT_TYPE>(new PRODUCT_TYPE)));

  std::string tag("rick");
  BKmap_t::const_iterator i(gf().branchKeys_.find(tag));
  assert(i != gf().branchKeys_.end());

  art::ProductList const& pl = art::ProductMetaData::instance().productList();
  art::ProductList::const_iterator it = pl.find(i->second);

  art::BranchDescription const& branchFromRegistry(it->second);

  std::shared_ptr<art::Parentage> entryDescriptionPtr(new art::Parentage);
  std::auto_ptr<art::ProductProvenance const>
    productProvenancePtr(new art::ProductProvenance(branchFromRegistry.branchID(),
                                                    art::productstatus::present(),
                                                    entryDescriptionPtr));

  art::ProcessConfiguration* process = gf().processConfigurations_[tag];
  assert(process);
  art::Timestamp now(1234567UL);
  art::RunAuxiliary runAux(eventID.run(), now, now);
  std::shared_ptr<art::RunPrincipal> rp(new art::RunPrincipal(runAux, *process));
  art::SubRunAuxiliary subRunAux(rp->run(), eventID.subRun(), now, now);
  std::shared_ptr<art::SubRunPrincipal>srp(new art::SubRunPrincipal(subRunAux, *process));
  srp->setRunPrincipal(rp);
  art::EventAuxiliary eventAux(eventID, now, true);
  pEvent_.reset(new art::EventPrincipal(eventAux, *process));
  pEvent_->setSubRunPrincipal(srp);
  pEvent_->put(product, branchFromRegistry, productProvenancePtr);

  assert(pEvent_->size() == 1u);
}

MPRGlobalTestFixture &
EventPrincipalTestFixture::gf() {
  static MPRGlobalTestFixture gf_s;
  return gf_s;
}



int work()
{
  ParameterSet reader_config;
  
  ProductRegistryHelper pr_helper;
  ProcessConfiguration process_config;
  PrincipalMaker principal_maker(process_config);
  
  RawEventQueueReader reader(reader_config,
                             pr_helper,
                             principal_maker);

  // Tell 'reader' the name of the file we are to read. This is pretty
  // much irrelevant for RawEventQueueReader, but we'll stick to the
  // interface demanded by ReaderSource<T>...
  string const fakeFileName("no such file exists");
  FileBlock* pFile(0);
  reader.readFile(fakeFileName, pFile);
  assert(pFile);
  assert(pFile->fileFormatVersion() == FileFormatVersion(1, "RawEvent2011"));
  assert(pFile->tree() == nullptr);
  assert(pFile->metaTree() == nullptr);
  assert(pFile->subRunTree() == nullptr);
  assert(pFile->subRunMetaTree() == nullptr);
  assert(pFile->runTree() == nullptr);
  assert(pFile->runMetaTree() == nullptr);
  assert(!pFile->fastClonable());

  // Test the end-of-data handling.
  artdaq::getGlobalQueue().enqNowait(std::shared_ptr<RawEvent>(nullptr)); // insert end-of-data marker
  RunID runid(2112);
  SubRunID subrunid(2112, 1);
  EventID eventid(2112, 1, 3);
  Timestamp now;

  std::unique_ptr<RunPrincipal>    run(principal_maker.makeRunPrincipal(runid.run(), now));
  std::unique_ptr<SubRunPrincipal> subrun(principal_maker.makeSubRunPrincipal(runid.run(), subrunid.subRun(), now));
  std::unique_ptr<EventPrincipal>  event(principal_maker.makeEventPrincipal(runid.run(),
                                                                            subrunid.subRun(),
                                                                            eventid.event(),
                                                                            now));

  EventPrincipal*  newevent;
  SubRunPrincipal* newsubrun;
  RunPrincipal*    newrun;
  bool rc = reader.readNext(run.get(), subrun.get(), newrun, newsubrun, newevent);
  assert(!rc);
  assert(newrun == nullptr);
  assert(newsubrun == nullptr);
  assert(newevent == nullptr);

  return 0;
}

int main()
{
  MPRGlobalTestFixture mpr_gtf;
  try {
    return work();
  }
  catch (art::Exception& x) {
    cerr << x << endl;
    return 1;
  }
}
