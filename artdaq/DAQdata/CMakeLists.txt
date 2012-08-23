set(LIB_NAME artdaq_DAQdata)
set(LIB_SOURCES
  Debug.cc
  Fragment.cc
  FragmentGenerator.cc
  RawEvent.cc
  makeFragmentGenerator.cc
  )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

target_link_libraries(${LIB_NAME}
  ${ART_PERSISTENCY_COMMON}
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${CETLIB}
  )

art_dictionary(DICTIONARY_LIBRARIES ${LIB_NAME})

simple_plugin(GenericFragmentSimulator "generator" ${LIB_NAME})

install_headers(SUBDIRS detail)
install_source()
install(TARGETS ${LIB_NAME}
  DESTINATION ${flavorqual_dir}/lib )

