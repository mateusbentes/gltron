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
include nebu/filesystem/CMakeFiles/nebu_filesystem.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include nebu/filesystem/CMakeFiles/nebu_filesystem.dir/compiler_depend.make

# Include the progress variables for this target.
include nebu/filesystem/CMakeFiles/nebu_filesystem.dir/progress.make

# Include the compile flags for this target's objects.
include nebu/filesystem/CMakeFiles/nebu_filesystem.dir/flags.make

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.o: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/flags.make
nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.o: nebu/filesystem/file_io.c
nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.o: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.o"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.o -MF CMakeFiles/nebu_filesystem.dir/file_io.c.o.d -o CMakeFiles/nebu_filesystem.dir/file_io.c.o -c /home/mateus/gltron/nebu/filesystem/file_io.c

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nebu_filesystem.dir/file_io.c.i"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/nebu/filesystem/file_io.c > CMakeFiles/nebu_filesystem.dir/file_io.c.i

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nebu_filesystem.dir/file_io.c.s"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/nebu/filesystem/file_io.c -o CMakeFiles/nebu_filesystem.dir/file_io.c.s

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.o: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/flags.make
nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.o: nebu/filesystem/filesystem.c
nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.o: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.o"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.o -MF CMakeFiles/nebu_filesystem.dir/filesystem.c.o.d -o CMakeFiles/nebu_filesystem.dir/filesystem.c.o -c /home/mateus/gltron/nebu/filesystem/filesystem.c

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nebu_filesystem.dir/filesystem.c.i"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/nebu/filesystem/filesystem.c > CMakeFiles/nebu_filesystem.dir/filesystem.c.i

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nebu_filesystem.dir/filesystem.c.s"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/nebu/filesystem/filesystem.c -o CMakeFiles/nebu_filesystem.dir/filesystem.c.s

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.o: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/flags.make
nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.o: nebu/filesystem/directory.c
nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.o: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.o"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.o -MF CMakeFiles/nebu_filesystem.dir/directory.c.o.d -o CMakeFiles/nebu_filesystem.dir/directory.c.o -c /home/mateus/gltron/nebu/filesystem/directory.c

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nebu_filesystem.dir/directory.c.i"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/nebu/filesystem/directory.c > CMakeFiles/nebu_filesystem.dir/directory.c.i

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nebu_filesystem.dir/directory.c.s"
	cd /home/mateus/gltron/nebu/filesystem && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/nebu/filesystem/directory.c -o CMakeFiles/nebu_filesystem.dir/directory.c.s

# Object files for target nebu_filesystem
nebu_filesystem_OBJECTS = \
"CMakeFiles/nebu_filesystem.dir/file_io.c.o" \
"CMakeFiles/nebu_filesystem.dir/filesystem.c.o" \
"CMakeFiles/nebu_filesystem.dir/directory.c.o"

# External object files for target nebu_filesystem
nebu_filesystem_EXTERNAL_OBJECTS =

nebu/filesystem/libnebu_filesystem.a: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/file_io.c.o
nebu/filesystem/libnebu_filesystem.a: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/filesystem.c.o
nebu/filesystem/libnebu_filesystem.a: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/directory.c.o
nebu/filesystem/libnebu_filesystem.a: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/build.make
nebu/filesystem/libnebu_filesystem.a: nebu/filesystem/CMakeFiles/nebu_filesystem.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C static library libnebu_filesystem.a"
	cd /home/mateus/gltron/nebu/filesystem && $(CMAKE_COMMAND) -P CMakeFiles/nebu_filesystem.dir/cmake_clean_target.cmake
	cd /home/mateus/gltron/nebu/filesystem && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nebu_filesystem.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
nebu/filesystem/CMakeFiles/nebu_filesystem.dir/build: nebu/filesystem/libnebu_filesystem.a
.PHONY : nebu/filesystem/CMakeFiles/nebu_filesystem.dir/build

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/clean:
	cd /home/mateus/gltron/nebu/filesystem && $(CMAKE_COMMAND) -P CMakeFiles/nebu_filesystem.dir/cmake_clean.cmake
.PHONY : nebu/filesystem/CMakeFiles/nebu_filesystem.dir/clean

nebu/filesystem/CMakeFiles/nebu_filesystem.dir/depend:
	cd /home/mateus/gltron && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mateus/gltron /home/mateus/gltron/nebu/filesystem /home/mateus/gltron /home/mateus/gltron/nebu/filesystem /home/mateus/gltron/nebu/filesystem/CMakeFiles/nebu_filesystem.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : nebu/filesystem/CMakeFiles/nebu_filesystem.dir/depend

