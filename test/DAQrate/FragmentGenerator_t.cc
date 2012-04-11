#define BOOST_TEST_MODULE ( FragmentGenerator_t )
#include "boost/test/auto_unit_test.hpp"

#include "artdaq/DAQdata/Fragment.hh"
#include "artdaq/DAQrate/FragmentGenerator.hh"

namespace artdaqtest {
  class FragmentGeneratorTest;
}

class artdaqtest::FragmentGeneratorTest :
  public artdaq::FragmentGenerator {
public:
  FragmentGeneratorTest();
private:
  bool getNext_(artdaq::FragmentPtrs &);
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
