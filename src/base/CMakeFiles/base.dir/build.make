# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mateus/gltron

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mateus/gltron

# Include any dependencies generated for this target.
include src/base/CMakeFiles/base.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/base/CMakeFiles/base.dir/compiler_depend.make

# Include the progress variables for this target.
include src/base/CMakeFiles/base.dir/progress.make

# Include the compile flags for this target's objects.
include src/base/CMakeFiles/base.dir/flags.make

src/base/CMakeFiles/base.dir/sdl_compat.c.o: src/base/CMakeFiles/base.dir/flags.make
src/base/CMakeFiles/base.dir/sdl_compat.c.o: src/base/sdl_compat.c
src/base/CMakeFiles/base.dir/sdl_compat.c.o: src/base/CMakeFiles/base.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/base/CMakeFiles/base.dir/sdl_compat.c.o"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/base/CMakeFiles/base.dir/sdl_compat.c.o -MF CMakeFiles/base.dir/sdl_compat.c.o.d -o CMakeFiles/base.dir/sdl_compat.c.o -c /home/mateus/gltron/src/base/sdl_compat.c

src/base/CMakeFiles/base.dir/sdl_compat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/base.dir/sdl_compat.c.i"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/src/base/sdl_compat.c > CMakeFiles/base.dir/sdl_compat.c.i

src/base/CMakeFiles/base.dir/sdl_compat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/base.dir/sdl_compat.c.s"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/src/base/sdl_compat.c -o CMakeFiles/base.dir/sdl_compat.c.s

src/base/CMakeFiles/base.dir/setcallback.c.o: src/base/CMakeFiles/base.dir/flags.make
src/base/CMakeFiles/base.dir/setcallback.c.o: src/base/setcallback.c
src/base/CMakeFiles/base.dir/setcallback.c.o: src/base/CMakeFiles/base.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object src/base/CMakeFiles/base.dir/setcallback.c.o"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/base/CMakeFiles/base.dir/setcallback.c.o -MF CMakeFiles/base.dir/setcallback.c.o.d -o CMakeFiles/base.dir/setcallback.c.o -c /home/mateus/gltron/src/base/setcallback.c

src/base/CMakeFiles/base.dir/setcallback.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/base.dir/setcallback.c.i"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/src/base/setcallback.c > CMakeFiles/base.dir/setcallback.c.i

src/base/CMakeFiles/base.dir/setcallback.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/base.dir/setcallback.c.s"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/src/base/setcallback.c -o CMakeFiles/base.dir/setcallback.c.s

src/base/CMakeFiles/base.dir/util.c.o: src/base/CMakeFiles/base.dir/flags.make
src/base/CMakeFiles/base.dir/util.c.o: src/base/util.c
src/base/CMakeFiles/base.dir/util.c.o: src/base/CMakeFiles/base.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object src/base/CMakeFiles/base.dir/util.c.o"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/base/CMakeFiles/base.dir/util.c.o -MF CMakeFiles/base.dir/util.c.o.d -o CMakeFiles/base.dir/util.c.o -c /home/mateus/gltron/src/base/util.c

src/base/CMakeFiles/base.dir/util.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/base.dir/util.c.i"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/src/base/util.c > CMakeFiles/base.dir/util.c.i

src/base/CMakeFiles/base.dir/util.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/base.dir/util.c.s"
	cd /home/mateus/gltron/src/base && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/src/base/util.c -o CMakeFiles/base.dir/util.c.s

# Object files for target base
base_OBJECTS = \
"CMakeFiles/base.dir/sdl_compat.c.o" \
"CMakeFiles/base.dir/setcallback.c.o" \
"CMakeFiles/base.dir/util.c.o"

# External object files for target base
base_EXTERNAL_OBJECTS =

src/base/libbase.a: src/base/CMakeFiles/base.dir/sdl_compat.c.o
src/base/libbase.a: src/base/CMakeFiles/base.dir/setcallback.c.o
src/base/libbase.a: src/base/CMakeFiles/base.dir/util.c.o
src/base/libbase.a: src/base/CMakeFiles/base.dir/build.make
src/base/libbase.a: src/base/CMakeFiles/base.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C static library libbase.a"
	cd /home/mateus/gltron/src/base && $(CMAKE_COMMAND) -P CMakeFiles/base.dir/cmake_clean_target.cmake
	cd /home/mateus/gltron/src/base && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/base.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/base/CMakeFiles/base.dir/build: src/base/libbase.a
.PHONY : src/base/CMakeFiles/base.dir/build

src/base/CMakeFiles/base.dir/clean:
	cd /home/mateus/gltron/src/base && $(CMAKE_COMMAND) -P CMakeFiles/base.dir/cmake_clean.cmake
.PHONY : src/base/CMakeFiles/base.dir/clean

src/base/CMakeFiles/base.dir/depend:
	cd /home/mateus/gltron && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mateus/gltron /home/mateus/gltron/src/base /home/mateus/gltron /home/mateus/gltron/src/base /home/mateus/gltron/src/base/CMakeFiles/base.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/base/CMakeFiles/base.dir/depend

