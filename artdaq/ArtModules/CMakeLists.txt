add_library(artdaq_ArtModules SHARED detail/RawEventQueueReader.cc)

target_link_libraries(artdaq_ArtModules
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
  )

simple_plugin(RawInput "source" artdaq_ArtModules)
