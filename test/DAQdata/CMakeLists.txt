
cet_test(FragmentGenerator_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata
  )

cet_test(GenericFragmentSimulator_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata
  )

set_source_files_properties(DS50Board_t.cc
  PROPERTIES
  COMPILE_FLAGS -Wno-overloaded-virtual
)

cet_test(DS50Board_t NO_AUTO USE_BOOST_UNIT
  LIBRARIES artdaq_DAQdata
)

cet_test(DS50Board_01_t HANDBUILT
  TEST_EXEC DS50Board_t
  TEST_PROPERTIES WILL_FAIL true
)

cet_test(DS50Board_02_t HANDBUILT
  TEST_EXEC DS50Board_t
  TEST_ARGS ${CMAKE_CURRENT_SOURCE_DIR}/ds50_frag0.bin
  )

cet_test(DS50Board_03_t HANDBUILT
  TEST_EXEC DS50Board_t
  TEST_ARGS
  ${CMAKE_CURRENT_SOURCE_DIR}/ds50_frag0.bin
  ${CMAKE_CURRENT_SOURCE_DIR}/ds50_frag1.bin
  ${CMAKE_CURRENT_SOURCE_DIR}/ds50_frag2.bin
  ${CMAKE_CURRENT_SOURCE_DIR}/ds50_frag3.bin
  ${CMAKE_CURRENT_SOURCE_DIR}/ds50_frag4.bin
  )
