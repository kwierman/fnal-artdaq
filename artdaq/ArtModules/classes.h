#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Persistency/Provenance/ParentageID.h"

template class std::pair<art::ParentageID, art::Parentage>;

namespace {
  struct dictionary {
    art::ParentageMap pm;
  };
}
