include(CetTest)

link_libraries(artdaq_Compression)

#cet_test(compress_t)

cet_test(compress_t DATAFILES thirty-two-12-bit-table.txt)
cet_test(step_1 HANDBUILT
  TEST_EXEC MakeSample
  TEST_ARGS 0 4 1000000 sample.training
  )

cet_test(step_1b HANDBUILT
  TEST_EXEC MakeSample
  TEST_ARGS 0 4 32769 sample.data
  )

cet_test(step_2 HANDBUILT
  TEST_EXEC BuildHuffmanTable
  TEST_ARGS ../step_1.d/sample.training table.txt
  TEST_PROPERTIES DEPENDS step_1
  )

cet_test(step_3 HANDBUILT
  TEST_EXEC EncodeHuffman
  TEST_ARGS ../step_2.d/table.txt ../step_1b.d/sample.data compressed.data
  )

set_tests_properties(step_3
  # If specified as part of cet_test, CMake imposes awkward escaping
  # requirements on lists, so do it this way.
  PROPERTIES DEPENDS "step_1b;step_2"
  )

cet_test(step_4 HANDBUILT
  TEST_EXEC DecodeHuffman
  TEST_ARGS ../step_2.d/table.txt ../step_3.d/compressed.data uncompressed.data
  TEST_PROPERTIES DEPENDS step_3
  )

cet_test(step_5 HANDBUILT
  TEST_EXEC diff
  TEST_ARGS -q ../step_1b.d/sample.data ../step_4.d/uncompressed.data
  TEST_PROPERTIES DEPENDS step_4
  )
