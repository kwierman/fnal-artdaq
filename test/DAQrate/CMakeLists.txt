if(WANT_MPI)
  include_directories(${MPI_INCLUDE_PATH})

  cet_test(config_t LIBRARIES artdaq_DAQrate artdaq_DAQdata)

  cet_test(config_with_art_t LIBRARIES artdaq_DAQrate artdaq_DAQdata
    TEST_PROPERTIES ENVIRONMENT ARTDAQ_DAQRATE_USE_ART=1
    )

  set(CPI_SOURCE ${MPI_INCLUDE_PATH}/../share/examples_graphics/cpi.c)
  if (EXISTS ${CPI_SOURCE})
    cet_test(cpi_t SOURCES ${CPI_SOURCE} LIBRARIES ${MPI_LIBRARIES})
  endif()

cet_test(daqrate_gen_test HANDBUILT
  TEST_EXEC daqrate
  DATAFILES fcl/daqrate_gen_test.fcl
  TEST_ARGS 5 1 4 11 -- -c daqrate_gen_test.fcl HANDBUILT
  TEST_PROPERTIES RUN_SERIAL true
  )
endif()
