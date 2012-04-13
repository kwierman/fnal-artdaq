#include "artdaq/DAQdata/DS50Board.hh"
#include "artdaq/DAQdata/DS50FragmentReader.hh"
#include "fhiclcpp/ParameterSet.h"

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;
#include <string>

int test_main(int, char **);

void bootstrap_test_main()
{
  int result = test_main(framework::master_test_suite().argc,
                         framework::master_test_suite().argv);
  BOOST_REQUIRE_EQUAL(result, 0);
}

bool init_unit_test()
{
  framework::master_test_suite().add(BOOST_TEST_CASE(&bootstrap_test_main));
  return true;
}

template <class InputIterator>
void
hexprint(std::ostream & os,
         InputIterator begin,
         InputIterator end,
         size_t items_per_line)
{
  std::size_t item_count = 0;
  std::ios_base::fmtflags old_fmt = os.setf(std::ios::hex, std::ios::basefield);
  char old_fill = os.fill('0');
  std::for_each(begin,
                end,
  [&](decltype(*begin) v) {
    os << std::setw(sizeof(v) * 2)
    << v
    << ((++item_count % items_per_line) ? ' ' : '\n');
  });
  os.fill(old_fill);
  os.setf(old_fmt);
}

int readAll(std::vector<std::string> const & fileNames)
{
  fhicl::ParameterSet ps;
  ps.put("max_set_size_gib", 0.002);
  ps.put("fileNames", fileNames);
  ds50::FragmentReader reader(ps);
  artdaq::FragmentPtrs frags;
  while (reader.getNext(frags)) {
    // Keep going.
  }
  std::cout
      << "Read "
      << frags.size()
      << " frags from "
      << fileNames.size()
      << " files.\n";
for (auto & fp : frags) {
    // Pad the back end to check for overruns.
    fp->resize(fp->dataSize() + 1);
    assert(*(fp->dataEnd() - 1) == 0);;
    *(fp->dataEnd() - 1) = -1;
    std::cout
        << *fp; // Fragment print.
    ds50::Board b(*fp);
    // First 16 adc values.
    hexprint(std::cerr,
             b.dataBegin(),
             b.dataBegin() + 16,
             8);
    std::cout << b; // Board print.
    b.checkADCData();
  }
  return 0;
}

int test_main(int argc, char ** argv)
{
  int result = 0;
  if (argc < 2) {
    std::cerr << "ERROR: usage: "
              << argv[0]
              << " <DS50-data-file>+\n";
    result = 1;
  }
  else {
    std::vector<std::string> fileNames;
    while (--argc) {
      fileNames.push_back(*++argv);
    }
    result = readAll(fileNames);
  }
  return result;
}

int main(int argc, char ** argv)
{
  return unit_test_main(&init_unit_test, argc, argv);
}
