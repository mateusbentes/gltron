find_path(MikMod_INCLUDE_DIR mikmod.h
    PATHS /usr/include
    PATH_SUFFIXES mikmod
)

find_library(MikMod_LIBRARY mikmod
    PATHS /usr/lib/x86_64-linux-gnu
)

if(MikMod_INCLUDE_DIR AND MikMod_LIBRARY)
    set(MikMod_FOUND TRUE)
    set(MikMod_INCLUDE_DIRS ${MikMod_INCLUDE_DIR})
    set(MikMod_LIBRARIES ${MikMod_LIBRARY})
else()
    set(MikMod_FOUND FALSE)
endif()

mark_as_advanced(MikMod_INCLUDE_DIR MikMod_LIBRARY)
