include_directories(${MPI_INCLUDE_PATH})
cet_test(daqrate PREBUILT NO_AUTO INSTALL_BIN)
cet_test(builder.sh PREBUILT NO_AUTO INSTALL_BIN)

cet_script(numawrap)

# ----------------------------------------------------------------------
# Build this project's executables:

# Nominally a test exec (no MPI) but we want this to be available to other packages.
art_make_exec(driver
  LIBRARIES
  artdaq_DAQrate
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Generators
  ${ART_FRAMEWORK_ART}
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )

art_make_exec(perfdump
  LIBRARIES
  artdaq_DAQrate
  )

art_make_exec(NAME builder
  SOURCE
  builder.cc
  Config.cc
  LIBRARIES
  ${ART_FRAMEWORK_ART}
  artdaq_DAQrate
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Generators
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${MPI_C_LIBRARIES}
  )

cet_test(config_t
  SOURCES
  config_t.cc
  Config.cc
  LIBRARIES artdaq_DAQrate )

cet_test(config_with_art_t
  SOURCES
  config_with_art_t.cc
  Config.cc
  LIBRARIES artdaq_DAQrate
  TEST_PROPERTIES ENVIRONMENT ARTDAQ_DAQRATE_USE_ART=1
  )

install_fhicl(LIST daqrate_simdata_noart.fcl)
install_source()
