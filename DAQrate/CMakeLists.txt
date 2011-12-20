# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

SET(MPI_CXX_COMPILER "mpicxx")
find_package(MPI REQUIRED)
SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})

set( LIB_NAME artdaq_DAQrate )

set( LIB_SOURCES 
     Config.cc
     Debug.cc
     EventPool.cc
     EventStore.cc
     Mpi.cc
     Perf.cc
     RHandles.cc
     SHandles.cc
     SimpleQueueReader.cc
     Utils.cc     
   )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

include(CetTest)
cet_test(daqrate PREBUILT NO_AUTO)

# ----------------------------------------------------------------------
# Build this project's executables:

link_libraries( ${LIB_NAME} )

add_executable( perfdump perfdump.cc )
add_executable( builder builder.cc )

# ----------------------------------------------------------------------
# Install this project's headers, sources, library, and executables:

install_headers()
install_source()
install( TARGETS ${LIB_NAME}
         DESTINATION ${flavorqual_dir}/lib )

#
# ======================================================================
