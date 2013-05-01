#define BOOST_TEST_MODULE ( FragmentGenerator_t )
#include "boost/test/auto_unit_test.hpp"

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQdata/FragmentGenerator.hh"

namespace artdaqtest {
  class FragmentGeneratorTest;
}

class artdaqtest::FragmentGeneratorTest :
  public artdaq::FragmentGenerator {
public:
  FragmentGeneratorTest();
  bool getNext_(artdaq::FragmentPtrs &) override;
  std::vector<artdaq::Fragment::fragment_id_t> fragmentIDs_() override;
  bool requiresStateMachine_() const override;
  void start_() override;
  void stop_() override;
  void pause_() override;
  void resume_() override;
};

artdaqtest::FragmentGeneratorTest::FragmentGeneratorTest()

  :
  FragmentGenerator()
{
}

bool
artdaqtest::FragmentGeneratorTest::getNext_(artdaq::FragmentPtrs & frags)
{
  frags.emplace_back(new artdaq::Fragment);
  return true;
}

std::vector<artdaq::Fragment::fragment_id_t>
artdaqtest::FragmentGeneratorTest::
fragmentIDs_()
{
  return { 1 };
}

bool
artdaqtest::FragmentGeneratorTest::requiresStateMachine_() const {
  return false;
}

void
artdaqtest::FragmentGeneratorTest::start_()
{ }

void
artdaqtest::FragmentGeneratorTest::stop_()
{ }

void
artdaqtest::FragmentGeneratorTest::pause_()
{ }

void
artdaqtest::FragmentGeneratorTest::resume_()
{ }

BOOST_AUTO_TEST_SUITE(FragmentGenerator_t)

BOOST_AUTO_TEST_CASE(Simple)
{
  artdaqtest::FragmentGeneratorTest testGen;
  artdaq::FragmentGenerator & baseGen(testGen);
  artdaq::FragmentPtrs fps;
  baseGen.getNext(fps);
  BOOST_REQUIRE_EQUAL(fps.size(), 1u);
}

BOOST_AUTO_TEST_SUITE_END()
