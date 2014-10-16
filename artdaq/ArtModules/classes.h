#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Persistency/Provenance/ParentageID.h"
#include "art/Persistency/Common/Wrapper.h"

namespace {
  struct dictionary {
    std::pair<const art::ParentageID, art::Parentage> pp;
    art::ParentageMap pm;
  };
}
