include(CetTest)


cet_test(config_t LIBRARIES artdaq_DAQrate artdaq_DAQdata)

cet_test(config_with_art_t LIBRARIES artdaq_DAQrate artdaq_DAQdata
  TEST_PROPERTIES ENVIRONMENT ARTDAQ_DAQRATE_USE_ART=1
  )

cet_test(FragmentGenerator_t USE_BOOST_UNIT
  LIBRARIES artdaq_DAQrate
  )

#cet_test(daqrate_trivial HANDBUILT
#         TEST_EXEC daqrate
#         TEST_ARGS 1 1 100 6000140 2 1
#         )

#cet_test(daqrate_simple HANDBUILT
#         TEST_EXEC daqrate
#         TEST_ARGS 5 5 100 6000140 2 1
#         )
