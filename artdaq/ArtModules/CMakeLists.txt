project(artdaq_bladder)

simple_plugin(DS50Compression "module" artdaq_Compression artdaq_DAQdata)
simple_plugin(DS50Decompression "module" artdaq_Compression artdaq_DAQdata)
simple_plugin(DS50DataCharacterizer "module" artdaq_DAQdata)

find_library( CETLIB NAMES cetlib PATHS $ENV{CETLIB_LIB} )
find_library( FHICLCPP NAMES fhicl_cpp PATHS $ENV{FHICLCPP_LIB} )
