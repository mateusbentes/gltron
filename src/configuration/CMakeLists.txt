
# Collect all C source files in this directory
file(GLOB CONFIGURATION_SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")


# Explicitly add settings.c if it's not already included
list(FIND CONFIGURATION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/settings.c" SETTINGS_INDEX)
if(SETTINGS_INDEX EQUAL -1)
  list(APPEND CONFIGURATION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/settings.c")
endif()

# Create a static library from the source files
add_library(configuration STATIC ${CONFIGURATION_SOURCES} ${HEADERS})

# Set include directories for this library
target_include_directories(configuration PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include        # Project-wide headers
  ${CMAKE_SOURCE_DIR}/nebu/include       # Nebu engine headers

)

# Define platform-specific macros
if(MACOSX)
  target_compile_definitions(configuration PRIVATE MACOSX)
elseif(WIN32)
  target_compile_definitions(configuration PRIVATE WIN32)
elseif(BUILD_ANDROID)
  target_compile_definitions(configuration PRIVATE ANDROID)
else()
  target_compile_definitions(configuration PRIVATE LINUX)
endif()

# Always enable touch support
target_compile_definitions(configuration PRIVATE ENABLE_TOUCH_SUPPORT)
