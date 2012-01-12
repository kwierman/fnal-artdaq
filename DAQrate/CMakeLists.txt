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
     GlobalQueue.cc
     Config.cc
     Debug.cc
     EventStore.cc
     FragmentPool.cc
     MPIProg.cc
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

find_library( ART_FRAMEWORK_CORE NAMES art_Framework_Core PATHS $ENV{ART_LIB} )

set ( common_exec_link_libraries
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

find_library( ART_PERSISTENCY_COMMON_DICT NAMES art_Persistency_Common_dict PATHS $ENV{ART_LIB} )
find_library( ART_FRAMEWORK_ART NAMES art_Framework_Art PATHS $ENV{ART_LIB} )
find_library( ART_FRAMEWORK_SERVICES_REGISTRY NAMES art_Framework_Services_Registry PATHS $ENV{ART_LIB} )
find_library( ART_UTILITIES NAMES art_Utilities PATHS $ENV{ART_LIB} )


set ( art_exec_link_libraries
  ${ART_PERSISTENCY_COMMON_DICT}
  ${ART_FRAMEWORK_ART}
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${ART_UTILITIES}
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
)

#target_link_libraries( artdaq_DAQrate
#                       ${art_exec_link_libraries}
#                       ${common_exec_link_libraries} )

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
