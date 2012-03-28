# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

find_package(MPI REQUIRED)
# Remove cxx library -- compiled with wrong version of GCC.
LIST(REMOVE_AT MPI_LIBRARIES 0)
include_directories(${MPI_INCLUDE_PATH})

set( LIB_NAME artdaq_DAQrate )
find_library( FHICLCPP NAMES fhiclcpp PATHS $ENV{FHICLCPP_LIB} )

string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
endif()

set( LIB_SOURCES 
     GlobalQueue.cc
     Config.cc
     Debug.cc
     FragmentGenerator.cc
     DS50FragmentReader.cc
     DS50FragmentSimulator.cc
     GenericFragmentSimulator.cc
     EventStore.cc
     MonitoredQuantity.cc
     MPIProg.cc
     Perf.cc
     RHandles.cc
     SHandles.cc
     SimpleQueueReader.cc
     StatisticsCollection.cc
     Utils.cc     
   )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

target_link_libraries( artdaq_DAQrate
  artdaq_DAQdata
  ${FHICLCPP}
  ${CLHEP}
  ${MPI_LIBRARIES}
  )

include(CetTest)
cet_test(daqrate PREBUILT NO_AUTO)

# ----------------------------------------------------------------------
# Build this project's executables:

find_library( ART_FRAMEWORK_CORE NAMES art_Framework_Core PATHS $ENV{ART_LIB} )

set ( common_exec_link_libraries
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
  ${SIGC}
  ${Boost_FILESYSTEM_LIBRARY}
)

#target_link_libraries( artdaq_DAQrate
#                       ${art_exec_link_libraries}
#                       ${common_exec_link_libraries} )

add_executable( perfdump perfdump.cc )
target_link_libraries(perfdump ${LIB_NAME})

add_executable( builder builder.cc )
target_link_libraries(builder
  ${LIB_NAME}
  ${art_exec_link_libraries}
  ${FHICLCPP}
  ${common_exec_link_libraries}
)

# ----------------------------------------------------------------------
# Install this project's headers, sources, library, and executables:

install_headers()
install_source()
install( TARGETS ${LIB_NAME}
         DESTINATION ${flavorqual_dir}/lib )

#
# ======================================================================
