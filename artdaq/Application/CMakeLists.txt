include(ProcessSmc)

process_smc(SMC_LIB_SOURCE Commandable.sm)

art_make_library(SOURCE
  ${SMC_LIB_SOURCE}
  Commandable.cc
  configureMessageFacility
  CommandableFragmentGenerator
  makeCommandableFragmentGenerator
  LIBRARIES
  artdaq_DAQrate
  ${ART_UTILITIES}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${FHICLCPP}
  )

install_headers()
install_source()

add_subdirectory(MPI2)
