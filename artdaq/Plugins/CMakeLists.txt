art_make(
  LIB_LIBRARIES
  ${ART_PERSISTENCY_COMMON}
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${CETLIB}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  )

simple_plugin("file" "metric"
  artdaq_Plugins
)

install_headers()
install_source()
