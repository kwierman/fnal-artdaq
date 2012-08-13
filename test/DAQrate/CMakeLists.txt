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

  cet_test(daqrate_fileread_nosink HANDBUILT
    TEST_EXEC daqrate
    DATAFILES fcl/daqrate_fileread.fcl
    fcl/daqrate_fileread_nosink.fcl
    TEST_ARGS 5 0 3 1 -- -c daqrate_fileread_nosink.fcl HANDBUILT
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(daqrate_fileread HANDBUILT
    TEST_EXEC daqrate
    DATAFILES fcl/daqrate_fileread.fcl
    TEST_ARGS 5 1 3 1 -- -c daqrate_fileread.fcl HANDBUILT
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(daqrate_perf_fileread_nosink_01_t
    HANDBUILT
    OPTIONAL_GROUPS PERF;LONG
    TEST_EXEC builder.sh
    TEST_ARGS -n -d 5 hosts-5-5-0.txt daqrate_perf_fileread_nosink.fcl
    DATAFILES hosts-5-5-0.txt
    fcl/daqrate_perf_fileread_nosink.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(daqrate_perf_fileread_nosink_02_t
    HANDBUILT
    OPTIONAL_GROUPS PERF;LONG
    TEST_EXEC builder.sh
    TEST_ARGS -n -d 5 hosts-5-5-0-mix.txt daqrate_perf_fileread_nosink.fcl
    DATAFILES hosts-5-5-0-mix.txt
    fcl/daqrate_perf_fileread_nosink.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(daqrate_perf_fileread_nosink_03_t
    HANDBUILT
    OPTIONAL_GROUPS PERF;LONG
    TEST_EXEC builder.sh
    TEST_ARGS -n -d 1 hosts-1-1-0.txt daqrate_perf_fileread_nosink.fcl
    DATAFILES hosts-1-1-0.txt
    fcl/daqrate_perf_fileread_nosink.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(daqrate_perf_fileread_nosink_04_t
    HANDBUILT
    OPTIONAL_GROUPS PERF;LONG
    TEST_EXEC builder.sh
    TEST_ARGS -n -d 1 hosts-1-1-0-mix.txt daqrate_perf_fileread_nosink.fcl
    DATAFILES hosts-1-1-0-mix.txt
    fcl/daqrate_perf_fileread_nosink.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(daqrate_perf_large_frag_nosink_01_t
    HANDBUILT
    OPTIONAL_GROUPS PERF;LONG
    TEST_EXEC builder.sh
    TEST_ARGS -n -d 1 hosts-1-1-0.txt daqrate_perf_large_frag_nosink.fcl
    DATAFILES hosts-1-1-0.txt
    fcl/daqrate_perf_large_frag_nosink.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(daqrate_perf_large_frag_nosink_02_t
    HANDBUILT
    OPTIONAL_GROUPS PERF;LONG
    TEST_EXEC builder.sh
    TEST_ARGS -n -d 1 hosts-1-1-0-mix.txt daqrate_perf_large_frag_nosink.fcl
    DATAFILES hosts-1-1-0-mix.txt
    fcl/daqrate_perf_large_frag_nosink.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )
endif()
