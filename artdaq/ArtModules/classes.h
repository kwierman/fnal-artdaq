#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Persistency/Provenance/ParentageID.h"
#include "art/Persistency/Common/Wrapper.h"
#include "artdaq/ArtModules/pkginfo.hh"

template class std::pair<art::ParentageID, art::Parentage>;
template class art::Wrapper<std::vector<artdaq::pkginfo> >;

namespace {
  struct dictionary {
    art::ParentageMap pm;
  };
}
