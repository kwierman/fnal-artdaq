art_make(
  EXCLUDE GenericFragmentSimulator_generator.cc
  LIB_LIBRARIES
  ${ARTDAQ-CORE_DATA}
  ${ARTDAQ-GENERATORS_GENERATORS}
  ${ART_PERSISTENCY_COMMON}
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${CETLIB}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  )

simple_plugin(GenericFragmentSimulator "generator"
  artdaq_DAQdata ${ARTDAQ-CORE_DATA} ${ARTDAQ-GENERATORS_GENERATORS}
  )

install_headers()
install_source()
