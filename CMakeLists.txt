# ======================================================================
#  artdaq main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -Dqualifier:STRING=$CETPKG_QUAL
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package
# ======================================================================

# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)

PROJECT(artdaq)

# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules
		      $ENV{ART_DIR}/Modules
		      $ENV{CETBUILDTOOLS_DIR}/Modules
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

# We are not yet ready to spend the time necessary to get the build to work with NO_UNDEFINED
cet_set_compiler_flags(DIAGS VIGILANT WERROR NO_UNDEFINED EXTRA_FLAGS -pedantic)
#cet_set_compiler_flags(DIAGS VIGILANT WERROR EXTRA_FLAGS -pedantic)

cet_have_qual( e2 )
if ( ${CET_HAVE_QUAL} )
  # -D_GLIBCXX_USE_NANOSLEEP is needed to make std::this_thread::sleep_for available.
  cet_add_compiler_flags(CXX -std=c++11 -fopenmp -D_GLIBCXX_USE_NANOSLEEP)
endif()
cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( art v1_00_12 )

find_ups_product( cetbuildtools v2_00_00 )

find_ups_product( cpp0x v1_03_00 )
find_ups_product( cetlib v1_03_00 )
find_ups_product( fhiclcpp v2_16_00 )
find_ups_product( messagefacility v1_10_00 )

find_ups_root( v5_30_02 )
find_ups_boost( v1_47_0 filesystem
                        program_options
                        regex
                        thread
                        unit_test_framework
 )

cet_check_gcc()

# include search path
include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}")
# notice use of ENV to invoke an external variable
include_directories ( $ENV{LIBSIGCPP_INC}/sigc++-2.0 )
include_directories ( $ENV{LIBSIGCPP_LIB}/sigc++-2.0/include )

#  Find all the libraries needed by our dependent CMakeList.txt files
find_library( ART_FRAMEWORK_ART NAMES art_Framework_Art PATHS $ENV{ART_LIB} )
find_library( ART_FRAMEWORK_CORE NAMES art_Framework_Core PATHS $ENV{ART_LIB} )
find_library( ART_FRAMEWORK_SERVICES_REGISTRY NAMES art_Framework_Services_Registry PATHS $ENV{ART_LIB} )
find_library( ART_PERSISTENCY_COMMON NAMES art_Persistency_Common PATHS $ENV{ART_LIB} )
find_library( ART_PERSISTENCY_COMMON_DICT NAMES art_Persistency_Common_dict PATHS $ENV{ART_LIB} )
find_library( ART_UTILITIES NAMES art_Utilities PATHS $ENV{ART_LIB} )
find_library( CETLIB NAMES cetlib PATHS $ENV{CETLIB_LIB} )
find_library( CPPUNIT NAMES cppunit   PATHS $ENV{CPPUNIT_LIB}   NO_DEFAULT_PATH )
find_library( FHICLCPP NAMES fhiclcpp PATHS $ENV{FHICLCPP_LIB} )
find_library( MF_EXTENSIONS NAMES MF_ExtModules PATHS $ENV{MESSAGEFACILITY_LIB} )
find_library( MF_MESSAGELOGGER NAMES MF_MessageLogger PATHS $ENV{MESSAGEFACILITY_LIB} )
find_library( MF_MESSAGESERVICE NAMES MF_MessageService PATHS $ENV{MESSAGEFACILITY_LIB} )
find_library( MF_UTILITIES NAMES MF_Utilities PATHS $ENV{MESSAGEFACILITY_LIB} )
find_library( SIGC    NAMES sigc-2.0  PATHS $ENV{LIBSIGCPP_LIB} NO_DEFAULT_PATH )

# define common flags
# set( CMAKE_CXX_LINK_FLAGS "${CPPUNIT} -luuid -ldl" )

# Debug streamer.
string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
endif()

cet_have_qual( nompi )
if(NOT ${CET_HAVE_QUAL})
  set(WANT_MPI TRUE)
endif()

if(${WANT_MPI})
  find_package(MPI QUIET)
  if(MPI_FOUND)
    # Remove cxx library -- compiled with wrong version of GCC.
    LIST(REMOVE_AT MPI_LIBRARIES 0)
  endif()
endif()

message(STATUS "Project is ${product} ${version} ${qualifier}")

# macros for artdaq_dictionary and simple_plugin
include(ArtDictionary)
include(BuildPlugins)

# testing
add_subdirectory(test)

# Modules - temporary
add_subdirectory(Modules)

# source
add_subdirectory(artdaq)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
