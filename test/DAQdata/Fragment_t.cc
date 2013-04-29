#include "artdaq/DAQdata/Fragment.hh"

#define BOOST_TEST_MODULE(Fragment_t)
#include "boost/test/auto_unit_test.hpp"

struct MetadataTypeOne {
  uint64_t field1;
  uint32_t field2;
  uint32_t field3;
};

struct MetadataTypeTwo {
  uint64_t field1;
  uint32_t field2;
  uint32_t field3;
  uint64_t field4;
  uint16_t field5;
};

struct MetadataTypeHuge {
  uint64_t fields[300];
};

BOOST_AUTO_TEST_SUITE(Fragment_test)

BOOST_AUTO_TEST_CASE(Construct)
{
  // 01-Mar-2013, KAB - I'm constructing these tests based on the
  // constructors that I already see in the class, but I have to
  // wonder if these truly correspond to the behavior that we want.
  artdaq::Fragment f1;
  BOOST_REQUIRE_EQUAL(f1.dataSize(),   (size_t) 0);
  BOOST_REQUIRE_EQUAL(f1.size(),       (size_t) 3);
  BOOST_REQUIRE_EQUAL(f1.version(),    (artdaq::Fragment::version_t) 0);
  BOOST_REQUIRE_EQUAL(f1.type(),       (artdaq::Fragment::type_t) 0);
  BOOST_REQUIRE_EQUAL(f1.sequenceID(), (artdaq::Fragment::sequence_id_t) 0);
  BOOST_REQUIRE_EQUAL(f1.fragmentID(), (artdaq::Fragment::fragment_id_t) 0);
  BOOST_REQUIRE_EQUAL(f1.hasMetadata(),false);

  artdaq::Fragment f2(7);
  BOOST_REQUIRE_EQUAL(f2.dataSize(),   (size_t)  7);
  BOOST_REQUIRE_EQUAL(f2.size(),       (size_t) 10);
  BOOST_REQUIRE_EQUAL(f2.version(),    (artdaq::Fragment::version_t) 0);
  BOOST_REQUIRE(f2.type() == artdaq::Fragment::InvalidFragmentType);
  BOOST_REQUIRE(f2.sequenceID() == artdaq::Fragment::InvalidSequenceID);
  BOOST_REQUIRE(f2.fragmentID() == artdaq::Fragment::InvalidFragmentID);
  BOOST_REQUIRE_EQUAL(f2.hasMetadata(),false);

  artdaq::Fragment f3(101, 202);
  BOOST_REQUIRE_EQUAL(f3.dataSize(),   (size_t) 0);
  BOOST_REQUIRE_EQUAL(f3.size(),       (size_t) 3);
  BOOST_REQUIRE_EQUAL(f3.version(),    (artdaq::Fragment::version_t) 0);
  BOOST_REQUIRE(f3.type() == artdaq::Fragment::DataFragmentType);
  BOOST_REQUIRE_EQUAL(f3.sequenceID(), (artdaq::Fragment::sequence_id_t) 101);
  BOOST_REQUIRE_EQUAL(f3.fragmentID(), (artdaq::Fragment::fragment_id_t) 202);
  BOOST_REQUIRE_EQUAL(f3.hasMetadata(),false);

  // Verify that only "user" fragment types may be specified
  // in the constructor
  try {
    artdaq::Fragment frag(101, 202, 0);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    artdaq::Fragment frag(101, 202, 225);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    artdaq::Fragment frag(101, 202, 255);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    artdaq::Fragment frag(101, 202, artdaq::Fragment::InvalidFragmentType);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    artdaq::Fragment
      frag(101, 202, artdaq::detail::RawFragmentHeader::FIRST_SYSTEM_TYPE);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    artdaq::Fragment
      frag(101, 202, artdaq::detail::RawFragmentHeader::LAST_SYSTEM_TYPE);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    artdaq::Fragment
      fragA(101, 202, artdaq::detail::RawFragmentHeader::FIRST_USER_TYPE);
    artdaq::Fragment
      fragB(101, 202, artdaq::detail::RawFragmentHeader::LAST_USER_TYPE);
    artdaq::Fragment fragC(101, 202,   1);
    artdaq::Fragment fragD(101, 202,   2);
    artdaq::Fragment fragE(101, 202,   3);
    artdaq::Fragment fragF(101, 202, 100);
    artdaq::Fragment fragG(101, 202, 200);
    artdaq::Fragment fragH(101, 202, 224);
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should not have thrown exception");
  }
}

BOOST_AUTO_TEST_CASE(FragmentType)
{
  artdaq::Fragment frag(15);

  // test "user" fragment types
  try {
    frag.setUserType(0);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setUserType(225);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setUserType(255);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setUserType(artdaq::Fragment::InvalidFragmentType);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setUserType(artdaq::detail::RawFragmentHeader::FIRST_SYSTEM_TYPE);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setUserType(artdaq::detail::RawFragmentHeader::LAST_SYSTEM_TYPE);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setUserType(artdaq::detail::RawFragmentHeader::FIRST_USER_TYPE);
    frag.setUserType(artdaq::detail::RawFragmentHeader::LAST_USER_TYPE);
    frag.setUserType(  1);
    frag.setUserType(  2);
    frag.setUserType(  3);
    frag.setUserType(100);
    frag.setUserType(200);
    frag.setUserType(224);
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should not have thrown exception");
  }

  // test "system" fragment types
  try {
    frag.setSystemType(0);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setSystemType(1);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setSystemType(224);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setSystemType(artdaq::Fragment::InvalidFragmentType);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setSystemType(artdaq::detail::RawFragmentHeader::FIRST_USER_TYPE);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setSystemType(artdaq::detail::RawFragmentHeader::LAST_USER_TYPE);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    frag.setSystemType(artdaq::detail::RawFragmentHeader::FIRST_SYSTEM_TYPE);
    frag.setSystemType(artdaq::detail::RawFragmentHeader::LAST_SYSTEM_TYPE);
    frag.setSystemType(225);
    frag.setSystemType(230);
    frag.setSystemType(240);
    frag.setSystemType(250);
    frag.setSystemType(255);
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should not have thrown exception");
  }
}

BOOST_AUTO_TEST_CASE(SequenceID)
{
  artdaq::Fragment f1;
  f1.setSequenceID(0);
  BOOST_REQUIRE_EQUAL(f1.sequenceID(), (uint64_t)0);
  f1.setSequenceID(1);
  BOOST_REQUIRE_EQUAL(f1.sequenceID(), (uint64_t)1);
  f1.setSequenceID(0xffff);
  BOOST_REQUIRE_EQUAL(f1.sequenceID(), (uint64_t)0xffff);
  f1.setSequenceID(0x0000ffffffffffff);
  BOOST_REQUIRE_EQUAL(f1.sequenceID(), (uint64_t)0x0000ffffffffffff);

  artdaq::Fragment f2(0x12345, 0xab);
  BOOST_REQUIRE_EQUAL(f2.sequenceID(), (uint64_t)0x12345);

  artdaq::Fragment f3(0x0000567812345678, 0xab);
  BOOST_REQUIRE_EQUAL(f3.sequenceID(), (uint64_t)0x0000567812345678);
}

BOOST_AUTO_TEST_CASE(FragmentID)
{
  artdaq::Fragment f1;
  f1.setFragmentID(0);
  BOOST_REQUIRE_EQUAL(f1.fragmentID(), (uint16_t)0);
  f1.setFragmentID(1);
  BOOST_REQUIRE_EQUAL(f1.fragmentID(), (uint16_t)1);
  f1.setFragmentID(0xffff);
  BOOST_REQUIRE_EQUAL(f1.fragmentID(), (uint16_t)0xffff);

  artdaq::Fragment f2(0x12345, 0xab);
  BOOST_REQUIRE_EQUAL(f2.fragmentID(), (uint16_t)0xab);

  artdaq::Fragment f3(0x0000567812345678, 0xffff);
  BOOST_REQUIRE_EQUAL(f3.fragmentID(), (uint16_t)0xffff);
}

BOOST_AUTO_TEST_CASE(Resize)
{
  // basic fragment
  artdaq::Fragment f1;
  f1.resize(1234);
  BOOST_REQUIRE_EQUAL(f1.dataSize(), (size_t) 1234);
  BOOST_REQUIRE_EQUAL(f1.size(), (size_t) 1234 +
                      artdaq::detail::RawFragmentHeader::num_words());

  // fragment with metadata
  MetadataTypeOne mdOneA;
  artdaq::Fragment f2(1, 123, 3, 5, mdOneA);
  f2.resize(129);
  BOOST_REQUIRE_EQUAL(f2.dataSize(), (size_t) 129);
  BOOST_REQUIRE_EQUAL(f2.size(), (size_t) 129 + 2 +
                      artdaq::detail::RawFragmentHeader::num_words());
}

BOOST_AUTO_TEST_CASE(Empty)
{
  artdaq::Fragment f1;
  BOOST_REQUIRE_EQUAL(f1.empty(), true);
  f1.resize(1234);
  BOOST_REQUIRE_EQUAL(f1.empty(), false);

  MetadataTypeOne mdOneA;
  artdaq::Fragment f2(1, 123, 3, 5, mdOneA);
  BOOST_REQUIRE_EQUAL(f2.empty(), false);
  f2.resize(129);
  BOOST_REQUIRE_EQUAL(f2.empty(), false);
  f2.resize(0);
  BOOST_REQUIRE_EQUAL(f2.empty(), true);
  BOOST_REQUIRE_EQUAL(f2.dataSize(), (size_t) 0);
  BOOST_REQUIRE_EQUAL(f2.size(), (size_t) 2 +
                      artdaq::detail::RawFragmentHeader::num_words());

  artdaq::Fragment f3;
  BOOST_REQUIRE_EQUAL(f3.empty(), true);
  f3.setMetadata(mdOneA);
  BOOST_REQUIRE_EQUAL(f3.empty(), true);

  artdaq::Fragment f4(14);
  BOOST_REQUIRE_EQUAL(f4.empty(), false);
  f4.setMetadata(mdOneA);
  BOOST_REQUIRE_EQUAL(f4.empty(), false);
}

BOOST_AUTO_TEST_CASE(Clear)
{
  artdaq::Fragment f1;
  BOOST_REQUIRE_EQUAL(f1.empty(), true);
  f1.resize(1234);
  BOOST_REQUIRE_EQUAL(f1.empty(), false);
  f1.clear();
  BOOST_REQUIRE_EQUAL(f1.empty(), true);

  MetadataTypeOne mdOneA;
  artdaq::Fragment f2(1, 123, 3, 5, mdOneA);
  BOOST_REQUIRE_EQUAL(f2.empty(), false);
  f2.resize(129);
  BOOST_REQUIRE_EQUAL(f2.empty(), false);
  f2.clear();
  BOOST_REQUIRE_EQUAL(f2.empty(), true);
  BOOST_REQUIRE_EQUAL(f2.dataSize(), (size_t) 0);
  BOOST_REQUIRE_EQUAL(f2.size(), (size_t) 2 +
                      artdaq::detail::RawFragmentHeader::num_words());

  artdaq::Fragment f3;
  BOOST_REQUIRE_EQUAL(f3.empty(), true);
  BOOST_REQUIRE_EQUAL(f3.hasMetadata(), false);
  f3.setMetadata(mdOneA);
  BOOST_REQUIRE_EQUAL(f3.empty(), true);
  BOOST_REQUIRE_EQUAL(f3.hasMetadata(), true);
  f3.clear();
  BOOST_REQUIRE_EQUAL(f3.empty(), true);
  BOOST_REQUIRE_EQUAL(f3.hasMetadata(), true);

  artdaq::Fragment f4(14);
  BOOST_REQUIRE_EQUAL(f4.empty(), false);
  BOOST_REQUIRE_EQUAL(f4.hasMetadata(), false);
  f4.setMetadata(mdOneA);
  BOOST_REQUIRE_EQUAL(f4.empty(), false);
  BOOST_REQUIRE_EQUAL(f4.hasMetadata(), true);
  f4.clear();
  BOOST_REQUIRE_EQUAL(f4.empty(), true);
  BOOST_REQUIRE_EQUAL(f4.hasMetadata(), true);
}

BOOST_AUTO_TEST_CASE(Addresses)
{
  // no metadata
  artdaq::Fragment f1(200);
  BOOST_REQUIRE_EQUAL(f1.dataSize(), (size_t) 200);
  BOOST_REQUIRE_EQUAL(f1.size(), (size_t) 200 +
                      artdaq::detail::RawFragmentHeader::num_words());
  artdaq::RawDataType* haddr = f1.headerAddress();
  artdaq::RawDataType* daddr = f1.dataAddress();
  BOOST_REQUIRE_EQUAL(daddr,
                      (haddr + artdaq::detail::RawFragmentHeader::num_words()));
  try {
    f1.metadataAddress();
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }
  BOOST_REQUIRE_EQUAL(haddr, &(*(f1.headerBegin())));
  BOOST_REQUIRE_EQUAL(daddr, &(*(f1.dataBegin())));
  BOOST_REQUIRE_EQUAL(daddr+200, &(*(f1.dataEnd())));

  // metadata with integer number of longwords
  MetadataTypeOne mdOneA;
  artdaq::Fragment f2(135, 101, 0, 3, mdOneA);
  BOOST_REQUIRE_EQUAL(f2.dataSize(), (size_t) 135);
  BOOST_REQUIRE_EQUAL(f2.size(), (size_t) 135 + 2 +
                      artdaq::detail::RawFragmentHeader::num_words());
  haddr = f2.headerAddress();
  daddr = f2.dataAddress();
  artdaq::RawDataType* maddr = f2.metadataAddress();
  BOOST_REQUIRE_EQUAL(maddr, haddr +
                      artdaq::detail::RawFragmentHeader::num_words());
  BOOST_REQUIRE_EQUAL(daddr, maddr + 2);
  BOOST_REQUIRE_EQUAL(haddr, &(*(f2.headerBegin())));
  BOOST_REQUIRE_EQUAL(daddr, &(*(f2.dataBegin())));
  BOOST_REQUIRE_EQUAL(daddr+135, &(*(f2.dataEnd())));

  // metadata with fractional number of longwords
  MetadataTypeTwo mdTwoA;
  artdaq::Fragment f3(77, 101, 0, 3, mdTwoA);
  BOOST_REQUIRE_EQUAL(f3.dataSize(), (size_t) 77);
  BOOST_REQUIRE_EQUAL(f3.size(), (size_t) 77 + 4 +
                      artdaq::detail::RawFragmentHeader::num_words());
  haddr = f3.headerAddress();
  daddr = f3.dataAddress();
  maddr = f3.metadataAddress();
  BOOST_REQUIRE_EQUAL(maddr, haddr +
                      artdaq::detail::RawFragmentHeader::num_words());
  BOOST_REQUIRE_EQUAL(daddr, maddr + 4);
  BOOST_REQUIRE_EQUAL(haddr, &(*(f3.headerBegin())));
  BOOST_REQUIRE_EQUAL(daddr, &(*(f3.dataBegin())));
  BOOST_REQUIRE_EQUAL(daddr+77, &(*(f3.dataEnd())));
}

BOOST_AUTO_TEST_CASE(Metadata)
{
  artdaq::Fragment f1(42);
  BOOST_REQUIRE_EQUAL(f1.hasMetadata(),false);
  try {
    f1.metadata<MetadataTypeOne>();
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  MetadataTypeOne mdOneA;
  mdOneA.field1 =  5;
  mdOneA.field2 = 10;
  mdOneA.field3 = 15;
  f1.setMetadata(mdOneA);
  MetadataTypeOne* mdOnePtr = f1.metadata<MetadataTypeOne>();
  BOOST_REQUIRE_EQUAL(mdOnePtr->field1, (uint64_t) 5);
  BOOST_REQUIRE_EQUAL(mdOnePtr->field2, (uint32_t)10);
  BOOST_REQUIRE_EQUAL(mdOnePtr->field3, (uint32_t)15);

  try {
    MetadataTypeOne mdOneB;
    f1.setMetadata(mdOneB);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  MetadataTypeTwo mdTwoA;
  mdTwoA.field1 = 10;
  mdTwoA.field2 = 20;
  mdTwoA.field3 = 30;
  mdTwoA.field4 = 40;
  mdTwoA.field5 = 50;
  artdaq::Fragment f2(10, 1, 2, 3, mdTwoA);
  MetadataTypeTwo* mdTwoPtr = f2.metadata<MetadataTypeTwo>();
  BOOST_REQUIRE_EQUAL(mdTwoPtr->field1, (uint64_t)10);
  BOOST_REQUIRE_EQUAL(mdTwoPtr->field2, (uint32_t)20);
  BOOST_REQUIRE_EQUAL(mdTwoPtr->field3, (uint32_t)30);
  BOOST_REQUIRE_EQUAL(mdTwoPtr->field4, (uint32_t)40);
  BOOST_REQUIRE_EQUAL(mdTwoPtr->field5, (uint32_t)50);

  artdaq::Fragment f3(0xabcdef, 0xc3a5, 123);
  BOOST_REQUIRE_EQUAL(f3.sequenceID(), (uint32_t)0xabcdef);
  BOOST_REQUIRE_EQUAL(f3.fragmentID(), (uint16_t)0xc3a5);
  BOOST_REQUIRE_EQUAL(f3.type(),       (uint16_t)123);
  f3.resize(5);
  BOOST_REQUIRE_EQUAL(f3.sequenceID(), (uint32_t)0xabcdef);
  BOOST_REQUIRE_EQUAL(f3.fragmentID(), (uint16_t)0xc3a5);
  BOOST_REQUIRE_EQUAL(f3.type(),       (uint8_t) 123);
  artdaq::RawDataType * dataPtr = f3.dataAddress();
  dataPtr[0] = 0x12345678;
  dataPtr[1] = 0xabcd;
  dataPtr[2] = 0x456789ab;
  dataPtr[3] = 0x3c3c3c3c;
  dataPtr[4] = 0x5a5a5a5a;
  BOOST_REQUIRE_EQUAL(dataPtr[0], (uint64_t)0x12345678);
  BOOST_REQUIRE_EQUAL(dataPtr[1], (uint64_t)0xabcd);
  BOOST_REQUIRE_EQUAL(dataPtr[2], (uint64_t)0x456789ab);
  BOOST_REQUIRE_EQUAL(dataPtr[3], (uint64_t)0x3c3c3c3c);
  BOOST_REQUIRE_EQUAL(dataPtr[4], (uint64_t)0x5a5a5a5a);
  BOOST_REQUIRE_EQUAL(f3.sequenceID(), (uint32_t)0xabcdef);
  BOOST_REQUIRE_EQUAL(f3.fragmentID(), (uint16_t)0xc3a5);
  BOOST_REQUIRE_EQUAL(f3.type(),       (uint8_t) 123);
  MetadataTypeOne mdOneC;
  mdOneC.field1 = 505;
  mdOneC.field2 = 510;
  mdOneC.field3 = 515;
  f3.setMetadata(mdOneC);
  mdOnePtr = f3.metadata<MetadataTypeOne>();
  BOOST_REQUIRE_EQUAL(mdOnePtr->field1, (uint64_t)505);
  BOOST_REQUIRE_EQUAL(mdOnePtr->field2, (uint32_t)510);
  BOOST_REQUIRE_EQUAL(mdOnePtr->field3, (uint32_t)515);
  BOOST_REQUIRE_EQUAL(f3.sequenceID(), (uint32_t)0xabcdef);
  BOOST_REQUIRE_EQUAL(f3.fragmentID(), (uint16_t)0xc3a5);
  BOOST_REQUIRE_EQUAL(f3.type(),       (uint8_t) 123);
  dataPtr = f3.dataAddress();
  dataPtr[0] = 0x12345678;
  dataPtr[1] = 0xabcd;
  dataPtr[2] = 0x456789ab;
  dataPtr[3] = 0x3c3c3c3c;
  dataPtr[4] = 0x5a5a5a5a;
  BOOST_REQUIRE_EQUAL(dataPtr[0], (uint64_t)0x12345678);
  BOOST_REQUIRE_EQUAL(dataPtr[1], (uint64_t)0xabcd);
  BOOST_REQUIRE_EQUAL(dataPtr[2], (uint64_t)0x456789ab);
  BOOST_REQUIRE_EQUAL(dataPtr[3], (uint64_t)0x3c3c3c3c);
  BOOST_REQUIRE_EQUAL(dataPtr[4], (uint64_t)0x5a5a5a5a);

  MetadataTypeHuge mdHuge;
  artdaq::Fragment f4(19);
  BOOST_REQUIRE_EQUAL(f4.hasMetadata(),false);
  try {
    f4.setMetadata(mdHuge);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }

  try {
    artdaq::Fragment f5(127, 1, 2, 3, mdHuge);
    BOOST_REQUIRE(0 && "Should have thrown exception");
  }
  catch (cet::exception const & excpt) {
  }
  catch (...) {
    BOOST_REQUIRE(0 && "Should have thrown cet::exception");
  }
}

BOOST_AUTO_TEST_SUITE_END()
