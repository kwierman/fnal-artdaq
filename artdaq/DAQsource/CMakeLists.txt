add_library(artdaq_DAQsource SHARED detail/RawEventQueueReader.cc)

simple_plugin(RawInput "source" artdaq_DAQsource)

add_executable(driver driver.cc)
target_link_libraries(driver
  artdaq_DAQrate
  artdaq_DAQdata
  ${ART_FRAMEWORK_ART}
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
