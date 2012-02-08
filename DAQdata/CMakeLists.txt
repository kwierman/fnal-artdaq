# project name
project(artdaq_DAQdata_DAQdataDictionaries)

set( LIB_NAME artdaq_DAQdata)
set( LIB_SOURCES
     Fragment.cc
    )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

art_dictionary(DICTIONARY_LIBRARIES ${ARTDAQ})

install_headers()
install_source()
install( TARGETS ${LIB_NAME}
         DESTINATION ${flavorqual_dir}/lib )

