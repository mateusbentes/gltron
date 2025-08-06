# src/configuration/CMakeLists.txt
file(GLOB CONFIGURATION_SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Explicitly add settings.c if it's not already included
list(FIND CONFIGURATION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/settings.c" SETTINGS_INDEX)
if(SETTINGS_INDEX EQUAL -1)
  list(APPEND CONFIGURATION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/settings.c")
endif()

add_library(configuration STATIC ${CONFIGURATION_SOURCES} ${HEADERS})
add_common_includes(configuration)
add_common_platform_definitions(configuration)

# Always enable touch support
target_compile_definitions(configuration PRIVATE ENABLE_TOUCH_SUPPORT)