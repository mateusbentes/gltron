#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "lib3ds::lib3ds" for configuration "Release"
set_property(TARGET lib3ds::lib3ds APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(lib3ds::lib3ds PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/liblib3ds.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS lib3ds::lib3ds )
list(APPEND _IMPORT_CHECK_FILES_FOR_lib3ds::lib3ds "${_IMPORT_PREFIX}/lib/liblib3ds.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
