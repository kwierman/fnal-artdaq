if(WANT_MPI)
  include_directories(${MPI_INCLUDE_PATH})
endif()

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
    ${ROOT_CINTEX}
    ${ROOT_RIO}
    ${ROOT_CORE}
    ${ROOT_NET}
    ${ROOT_REFLEX}
)

simple_plugin(RawInput "source"
  artdaq_ArtModules
  ${ART_FRAMEWORK_CORE}
  ${ART_FRAMEWORK_IO_SOURCES}
  ${ART_FRAMEWORK_PRINCIPAL}
  ${ART_PERSISTENCY_PROVENANCE}
  ${ART_UTILITIES}
  ${MF_MESSAGELOGGER}
)

simple_plugin(NetMonInput "source"
  ${CETLIB}
  ${ART_FRAMEWORK_CORE}
  ${ART_FRAMEWORK_IO_SOURCES}	
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  artdaq_ArtModules
)

simple_plugin(NetMonOutput "module"
  ${ROOT_RIO}
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${MF_MESSAGELOGGER}
  artdaq_ArtModules
)

simple_plugin(NetMonTransportService "service"
  artdaq_DAQrate
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${ROOT_RIO}
  ${ROOT_NET}
  ${ROOT_CORE}
)
