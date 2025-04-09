# Add subdirectories for all components
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(video)
add_subdirectory(scripting)
add_subdirectory(filesystem)
add_subdirectory(base)

# Note: We've removed the problematic code that tried to link all nebu libraries with sdl_compat
# Instead, we'll link each library individually in its own CMakeLists.txt file
