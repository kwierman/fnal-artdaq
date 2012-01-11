include(CetTest)


cet_test(config_t LIBRARIES artdaq_DAQrate)


cet_test(daqrate_simple HANDBUILT
         TEST_EXEC daqrate
         TEST_ARGS 5 5 100 6000140 2 1
         )
