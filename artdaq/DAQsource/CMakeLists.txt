add_library(artdaq_DAQsource SHARED detail/RawEventQueueReader.cc)
simple_plugin(RawInput "source" artdaq_DAQsource)

target_link_libraries(artdaq_DAQsource
  artdaq_DAQrate
  artdaq_DAQdata
  ${CETLIB}
  ${FHICLCPP}
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${ART_FRAMEWORK_CORE}
  ${ROOT_CINTEX}
  ${ROOT_PHYSICS}
  ${ROOTSYS}/lib/libGraf.so
  ${ROOT_TREE}
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_NET}
  ${ROOT_MATHCORE}
  ${ROOT_RIO}
  ${ROOT_THREAD}
  ${ROOT_CORE}
  ${ROOT_CINT}
  ${ROOT_REFLEX}
)

if(WANT_MPI)
  add_executable(driver driver.cc)
  target_link_libraries(driver
    artdaq_DAQsource
    artdaq_DAQrate
    artdaq_DAQdata
    ${CETLIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${FHICLCPP}
    ${ART_PERSISTENCY_COMMON_DICT}
    ${ART_FRAMEWORK_ART}
    ${ART_FRAMEWORK_SERVICES_REGISTRY}
    ${ART_UTILITIES}
    ${SIGC}
    ${Boost_FILESYSTEM_LIBRARY}
    ${ART_FRAMEWORK_CORE}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${ROOT_CINTEX}
    ${ROOT_PHYSICS}
    ${ROOTSYS}/lib/libGraf.so
    ${ROOT_TREE}
    ${ROOT_HIST}
    ${ROOT_MATRIX}
    ${ROOT_NET}
    ${ROOT_MATHCORE}
    ${ROOT_RIO}
    ${ROOT_THREAD}
    ${ROOT_CORE}
    ${ROOT_CINT}
    ${ROOT_REFLEX}
    ${CPPUNIT}
    -ldl
    )
endif()



