project(artdaq_test)

simple_plugin(RawInput "source" artdaq_DAQrate)

add_executable(driver driver.cc )

find_library( CETLIB NAMES cetlib PATHS $ENV{CETLIB_LIB} )
find_library( FHICLCPP NAMES fhicl_cpp PATHS $ENV{FHICLCPP_LIB} )

target_link_libraries(driver artdaq_DAQrate
                      ${CETLIB}
                      ${Boost_PROGRAM_OPTIONS_LIBRARY}
                      ${FHICLCPP}
                      artdaq_DAQdata
                     )
