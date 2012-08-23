simple_plugin(FragmentSniffer "module"
  artdaq_DAQdata
  ${ART_FRAMEWORK_CORE}
  ${ART_FRAMEWORK_PRINCIPAL}
  ${ART_PERSISTENCY_PROVENANCE}
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  )

cet_test(raw_event_queue_reader_t USE_BOOST_UNIT
         LIBRARIES
         artdaq_ArtModules
         artdaq_DAQdata_dict
         artdaq_DAQrate
         artdaq_DAQdata
         ${ART_FRAMEWORK_CORE}
         ${ART_FRAMEWORK_PRINCIPAL}
         ${ART_PERSISTENCY_PROVENANCE}
         ${ART_UTILITIES}
         )

if(WANT_MPI)
  cet_test(daq_flow_t
    LIBRARIES
    ${ART_FRAMEWORK_ART}
    artdaq_DAQrate
    artdaq_DAQdata
    ${MF_MESSAGELOGGER}
    ${FHICLCPP}
    ${CETLIB}
    TEST_ARGS -c daq_flow_t.fcl
    DATAFILES daq_flow_t.fcl
    )
endif()

# Wait for an input for a second, then exit.
cet_test(read_files_t HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -c read_files_t.fcl
  DATAFILES read_files_t.fcl
  )

add_executable(driver driver.cc)
target_link_libraries(driver
  artdaq_DAQrate
  artdaq_DAQdata
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )

# Generate several fragments and verify they made it into the store.
cet_test(driver_t HANDBUILT
  TEST_EXEC driver
  TEST_ARGS -c driver_t.fcl
  DATAFILES driver_t.fcl
  )

