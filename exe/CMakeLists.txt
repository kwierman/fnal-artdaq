include(CetTest)
include(CheckUpsVersion)
 _check_if_version_greater(cetbuildtools ${CETBUILDTOOLS_VERSION} v2_01_05)

if(WANT_MPI)
  include_directories(${MPI_INCLUDE_PATH})
  cet_test(daqrate PREBUILT NO_AUTO INSTALL_BIN)
  cet_test(builder.sh PREBUILT NO_AUTO INSTALL_BIN)
endif()

cet_test(numawrap PREBUILT NO_AUTO INSTALL_BIN)

# ----------------------------------------------------------------------
# Build this project's executables:

if(WANT_MPI)
  add_executable( perfdump perfdump.cc )
  target_link_libraries(perfdump artdaq_DAQrate)

  add_executable( builder builder.cc )
  target_link_libraries(builder
    ${ART_FRAMEWORK_ART}
    artdaq_DAQrate
    artdaq_DAQdata
    ${FHICLCPP}
    ${CETLIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${MPI_LIBRARIES}
    )
  install(TARGETS perfdump builder DESTINATION ${flavorqual_dir}/bin)
endif()
