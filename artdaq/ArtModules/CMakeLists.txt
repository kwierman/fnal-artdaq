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

if (NOT ART_FRAMEWORK_IO_SOURCES)
  find_library(ART_FRAMEWORK_IO_SOURCES art_Framework_IO_Sources PATHS $ENV{ART_LIB} )
endif()

simple_plugin(RawInput "source" artdaq_ArtModules ${ART_FRAMEWORK_IO_SOURCES})

install(TARGETS ${LIB_NAME}
  DESTINATION ${flavorqual_dir}/lib )
