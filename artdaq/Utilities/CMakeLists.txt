# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

set( LIB_NAME artdaq_Utilities )

set(LIB_SOURCES
  SimpleLookupPolicy.cc
  )

set(LIB_LIBS
  ${CETLIB}
  ${FHICLCPP}
)

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES})

target_link_libraries(artdaq_Utilities ${LIB_LIBS})

# ----------------------------------------------------------------------
# Install this project's headers, sources, library, and executables:

install_headers(SUBDIRS detail)
install_source()
install( TARGETS ${LIB_NAME}
         DESTINATION ${flavorqual_dir}/lib )

#
# ======================================================================
