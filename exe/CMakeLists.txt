if(WANT_MPI)
  include_directories(${MPI_INCLUDE_PATH})
  cet_test(daqrate PREBUILT NO_AUTO INSTALL_BIN)
  cet_test(builder.sh PREBUILT NO_AUTO INSTALL_BIN)
endif()

cet_script(numawrap)

# ----------------------------------------------------------------------
# Build this project's executables:

# Nominally a test exec (no MPI) but we want this to be available to other packages.
art_make_exec(NAME driver
  LIBRARIES
  artdaq_DAQrate
  artdaq_DAQdata
  artdaq_Utilities
  ${ART_FRAMEWORK_ART}
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )

if(WANT_MPI)
  art_make_exec(NAME perfdump
    LIBRARIES
    artdaq_DAQrate
    )

  art_make_exec(NAME builder
    LIBRARIES
    ${ART_FRAMEWORK_ART}
    artdaq_DAQrate
    artdaq_DAQdata
    artdaq_Utilities
    ${FHICLCPP}
    ${CETLIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${MPI_LIBRARIES}
    )

  install_fhicl(LIST daqrate_simdata_noart.fcl)
endif()
