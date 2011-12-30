# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

#set( LIB_NAME ${PROJECT_NAME} )
set( LIB_NAME artdaq_Compression )

set( LIB_SOURCES 
     BlockReader.cc
     Decoder.cc
     Encoder.cc
     HuffmanTable.cc
     SymCode.cc
     SymProb.cc
     SymTable.cc
   )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

# ----------------------------------------------------------------------
# Build this project's executables:

link_libraries( ${LIB_NAME} )

add_executable( BuildHuffmanTable BuildHuffmanTable.cc )
add_executable( DecodeHuffman DecodeHuffman.cc )
add_executable( EncodeHuffman EncodeHuffman.cc )
add_executable( EncodeDS50 EncodeDS50.cc )
add_executable( BuildRiceTable BuildRiceTable.cc )
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
