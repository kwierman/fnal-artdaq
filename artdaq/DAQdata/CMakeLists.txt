art_make(
  EXCLUDE GenericFragmentSimulator_generator.cc
  LIB_LIBRARIES
  ${ART_PERSISTENCY_COMMON}
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${CETLIB}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  )

simple_plugin(GenericFragmentSimulator "generator" artdaq_DAQdata)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
