
set(LIB_NAME artdaq_DAQdata)
set(LIB_SOURCES
  Debug.cc
  Fragment.cc
  FragmentGenerator.cc
  GenericFragmentSimulator.cc
  RawEvent.cc
  makeFragmentGenerator.cc
  )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

target_link_libraries(${LIB_NAME}
  ${ART_PERSISTENCY_COMMON}
  ${FHICLCPP}
  ${CETLIB}
  )

install_headers(SUBDIRS detail)
install_source()
install(TARGETS ${LIB_NAME}
  DESTINATION ${flavorqual_dir}/lib )

