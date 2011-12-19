# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

#set( LIB_NAME ${PROJECT_NAME} )
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
     Utils.cc     
   )

link_libraries( mpi )
add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

# ----------------------------------------------------------------------
# Build this project's executables:

link_libraries( ${LIB_NAME} )

add_executable( builder builder.cc )
add_executable( perfdump perfdump.cc )
add_executable( rate rate.cc )

# ----------------------------------------------------------------------
# Install this project's headers, sources, library, and executables:

install_headers()
install_source()
install( TARGETS ${LIB_NAME}
         DESTINATION ${flavorqual_dir}/lib )

#
# ======================================================================
