# Automatically include all source files
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# Create the library
add_library(nebu_scripting STATIC ${SOURCES})

# Set include directories
target_include_directories(nebu_scripting PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/lua5/include
)

# Link against libraries
target_link_libraries(nebu_scripting PUBLIC
  lua5
  lualib5
)
