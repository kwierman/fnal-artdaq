////////////////////////////////////////////////////////////////////////
// Class:       DS50CompressionChecker
// Module Type: analyzer
// File:        DS50CompressionChecker_module.cc
//
// Generated at Mon Apr 16 11:46:47 2012 by Christopher Green using artmod
// from art v0_00_02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "artdaq/DAQdata/Fragments.hh"

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <cassert>

using artdaq::Fragment;
using artdaq::Fragments;

namespace ds50test {
  class DS50CompressionChecker;
}

class ds50test::DS50CompressionChecker : public art::EDAnalyzer {
public:
  explicit DS50CompressionChecker(fhicl::ParameterSet const & p);
  virtual ~DS50CompressionChecker();

  virtual void analyze(art::Event const & e);

private:

    std::string raw_label_;
    std::string uncompressed_label_;
};


ds50test::DS50CompressionChecker::DS50CompressionChecker(fhicl::ParameterSet const & p)
  :
  raw_label_(p.get<std::string>("raw_label")),
  uncompressed_label_(p.get<std::string>("uncompressed_label"))
{
}

ds50test::DS50CompressionChecker::~DS50CompressionChecker()
{
}

void ds50test::DS50CompressionChecker::analyze(art::Event const & e)
{
  art::Handle<artdaq::Fragments> raw, uncomp;
  assert(e.getByLabel(raw_label_, raw));
  assert(e.getByLabel(uncompressed_label_, uncomp));
  assert(raw->size() == uncomp->size());
  size_t len = raw->size();
#pragma omp parallel for shared(len, raw, uncomp)
  for (size_t i = 0; i < len; ++i) {
    Fragment const & rf ((*raw)[i]);
    Fragment const & uf ((*uncomp)[i]);
    assert(rf.size() == uf.size());
    assert(std::equal(rf.dataBegin(), rf.dataEnd(), uf.dataBegin()));
  }
}

DEFINE_ART_MODULE(ds50test::DS50CompressionChecker)
