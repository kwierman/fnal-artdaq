find_library( FHICLCPP NAMES fhiclcpp PATHS $ENV{FHICLCPP_LIB} )

set(LIB_NAME artdaq_DAQdata)
set(LIB_SOURCES
  DS50FragmentReader.cc
  DS50FragmentSimulator.cc
  DS50Board.cc
  DS50RawData.cc 
  Debug.cc
  Fragment.cc
  FragmentGenerator.cc
  GenericFragmentSimulator.cc
  RawEvent.cc
  )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

target_link_libraries(${LIB_NAME}
  ${FHICLCPP}
  ${CLHEP}
  )

art_dictionary(DICTIONARY_LIBRARIES)

install_headers(SUBDIRS detail)
install_source()
install(TARGETS ${LIB_NAME}
  DESTINATION ${flavorqual_dir}/lib )

