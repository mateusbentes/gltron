file(GLOB ANDROID_SOURCES "*.c" "*.cpp")
file(GLOB ANDROID_HEADERS "*.h")

if(ANDROID_SOURCES)
  add_library(gltron_android STATIC ${ANDROID_SOURCES} ${ANDROID_HEADERS})
  target_include_directories(gltron_android PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  # Link to Android native libraries if needed
  target_link_libraries(gltron_android PUBLIC ${ANDROID_LIB} ${LOG_LIB} ${EGL_LIB} ${GLES_LIB})
  message(STATUS "[android] Android-specific static library created with sources: ${ANDROID_SOURCES}")
else()
  add_library(gltron_android INTERFACE)
  message(STATUS "[android] No Android-specific sources found, created INTERFACE library")
endif()