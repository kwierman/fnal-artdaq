set(LIB_NAME artdaq_ArtModules)
add_library(${LIB_NAME} SHARED detail/RawEventQueueReader.cc)

target_link_libraries(${LIB_NAME}
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

install(TARGETS ${LIB_NAME}
  DESTINATION ${flavorqual_dir}/lib )
