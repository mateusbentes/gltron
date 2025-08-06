# lib3dsConfig.cmake
# Configuration file for the lib3ds package

# Find the lib3ds library
find_library(LIB3DS_LIBRARY
  NAMES lib3ds
  HINTS ${CMAKE_INSTALL_PREFIX}/lib
)

# Find the lib3ds include directory
find_path(LIB3DS_INCLUDE_DIR
  NAMES lib3ds.h
  HINTS ${CMAKE_INSTALL_PREFIX}/include
)

# Handle the QUIETLY and REQUIRED arguments and set LIB3DS_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(lib3ds
  REQUIRED_VARS LIB3DS_LIBRARY LIB3DS_INCLUDE_DIR
)

# Export the targets
if(LIB3DS_FOUND AND NOT TARGET lib3ds::lib3ds)
  add_library(lib3ds::lib3ds UNKNOWN IMPORTED)
  set_target_properties(lib3ds::lib3ds PROPERTIES
    IMPORTED_LOCATION "${LIB3DS_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LIB3DS_INCLUDE_DIR}"
  )
endif()

# Export the variables
mark_as_advanced(LIB3DS_INCLUDE_DIR LIB3DS_LIBRARY)