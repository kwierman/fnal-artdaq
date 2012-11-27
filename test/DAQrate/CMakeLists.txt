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

  add_executable(s_r_handles s_r_handles.cc)
  target_link_libraries(s_r_handles artdaq_DAQrate artdaq_DAQdata)

  # This is an integration test that exercises SHandles and RHandles,
  # making sure they can communicate as expected.
  set(num_sending_ranks 2)
  set(total_ranks 5)
  # there will be (total_ranks - num_sending_ranks) receiving ranks
  cet_test(s_r_handles_t HANDBUILT
    TEST_EXEC mpirun
    TEST_ARGS -hosts localhost -np ${total_ranks} s_r_handles ${num_sending_ranks}
  )

  cet_test(daqrate_gen_test HANDBUILT
    TEST_EXEC daqrate
    DATAFILES fcl/daqrate_gen_test.fcl
    TEST_ARGS 5 1 4 11 -- -c daqrate_gen_test.fcl
    TEST_PROPERTIES RUN_SERIAL true
    )

  cet_test(FragCounter_t USE_BOOST_UNIT
    LIBRARIES artdaq_DAQrate)

endif()
