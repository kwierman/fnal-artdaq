# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

#set( LIB_NAME ${PROJECT_NAME} )
set( LIB_NAME artdaq_Compression )

string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
endif()

set( LIB_SOURCES 
     BlockReader.cc
     Decoder.cc
     Encoder.cc
     HuffmanTable.cc
     SymProb.cc
     SymTable.cc
   )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

target_link_libraries(${LIB_NAME} artdaq_DAQrate artdaq_DAQdata ${CETLIB})

# ----------------------------------------------------------------------
# Build this project's executables:
link_libraries( ${LIB_NAME} ) # All execs need this.

add_executable( BuildHuffmanTable BuildHuffmanTable.cc )
add_executable( DecodeHuffman DecodeHuffman.cc )
add_executable( EncodeHuffman EncodeHuffman.cc )
#add_executable( EncodeDS50 EncodeDS50.cc )
add_executable( BuildRiceTable BuildRiceTable.cc )
add_executable( Codes Codes.cc )
add_executable( MakeSample MakeSample.cc )
add_executable( tester tester.cc )

# ----------------------------------------------------------------------
# Install this project's headers, sources, library, and executables:

install_headers()
install_source()
install( TARGETS ${LIB_NAME}
         DESTINATION ${flavorqual_dir}/lib )
install( TARGETS MakeSample
         DESTINATION ${flavorqual_dir}/bin )

#
# ======================================================================
