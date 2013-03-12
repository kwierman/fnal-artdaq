art_make_library(
  SOURCE 
    RawInput_source.cc
    NetMonTransportService_service.cc
    detail/RawEventQueueReader.cc
  LIBRARIES
    artdaq_DAQrate
    artdaq_DAQdata
    ${ART_FRAMEWORK_CORE}
    ${ART_FRAMEWORK_IO_SOURCES}
    ${ART_FRAMEWORK_PRINCIPAL}
    ${ART_PERSISTENCY_PROVENANCE}
    ${ART_UTILITIES}
    ${MF_MESSAGELOGGER}
    ${ROOT_CORE}
    ${ROOT_REFLEX}
    ${ROOT_NET}
)

simple_plugin(NetMonInput "source"
  ${CETLIB}
  ${ART_FRAMEWORK_CORE}
  ${ART_FRAMEWORK_IO_SOURCES}	
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${FHICLCPP}
  artdaq_ArtModules
)

simple_plugin(NetMonOutput "module"
  ${ROOT_RIO}
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  artdaq_ArtModules
)

simple_plugin(NetMonTransportService "service"
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${ROOT_NET}
  ${ROOT_CORE}
)
