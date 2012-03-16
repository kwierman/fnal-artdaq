project(artdaq_bladder)

# simple_plugin(RawInput "source" artdaq_DAQrate)
simple_plugin(DS50Compression "module" artdaq_ArtModule )

# add_executable(driver.cc )

find_library( CETLIB NAMES cetlib PATHS $ENV{CETLIB_LIB} )
find_library( FHICLCPP NAMES fhicl_cpp PATHS $ENV{FHICLCPP_LIB} )

#target_link_libraries(driver
#  artdaq_Compression
#  artdaq_DAQdata
#  ${CETLIB}
#  ${Boost_PROGRAM_OPTIONS_LIBRARY}
#  ${FHICLCPP}
#  )
