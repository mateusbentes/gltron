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
include nebu/input/CMakeFiles/nebu_input.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include nebu/input/CMakeFiles/nebu_input.dir/compiler_depend.make

# Include the progress variables for this target.
include nebu/input/CMakeFiles/nebu_input.dir/progress.make

# Include the compile flags for this target's objects.
include nebu/input/CMakeFiles/nebu_input.dir/flags.make

nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.o: nebu/input/CMakeFiles/nebu_input.dir/flags.make
nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.o: nebu/input/custom_keys.c
nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.o: nebu/input/CMakeFiles/nebu_input.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.o"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.o -MF CMakeFiles/nebu_input.dir/custom_keys.c.o.d -o CMakeFiles/nebu_input.dir/custom_keys.c.o -c /home/mateus/gltron/nebu/input/custom_keys.c

nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nebu_input.dir/custom_keys.c.i"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/nebu/input/custom_keys.c > CMakeFiles/nebu_input.dir/custom_keys.c.i

nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nebu_input.dir/custom_keys.c.s"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/nebu/input/custom_keys.c -o CMakeFiles/nebu_input.dir/custom_keys.c.s

nebu/input/CMakeFiles/nebu_input.dir/input_system.c.o: nebu/input/CMakeFiles/nebu_input.dir/flags.make
nebu/input/CMakeFiles/nebu_input.dir/input_system.c.o: nebu/input/input_system.c
nebu/input/CMakeFiles/nebu_input.dir/input_system.c.o: nebu/input/CMakeFiles/nebu_input.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object nebu/input/CMakeFiles/nebu_input.dir/input_system.c.o"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT nebu/input/CMakeFiles/nebu_input.dir/input_system.c.o -MF CMakeFiles/nebu_input.dir/input_system.c.o.d -o CMakeFiles/nebu_input.dir/input_system.c.o -c /home/mateus/gltron/nebu/input/input_system.c

nebu/input/CMakeFiles/nebu_input.dir/input_system.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nebu_input.dir/input_system.c.i"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/nebu/input/input_system.c > CMakeFiles/nebu_input.dir/input_system.c.i

nebu/input/CMakeFiles/nebu_input.dir/input_system.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nebu_input.dir/input_system.c.s"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/nebu/input/input_system.c -o CMakeFiles/nebu_input.dir/input_system.c.s

nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.o: nebu/input/CMakeFiles/nebu_input.dir/flags.make
nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.o: nebu/input/system_keynames.c
nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.o: nebu/input/CMakeFiles/nebu_input.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.o"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.o -MF CMakeFiles/nebu_input.dir/system_keynames.c.o.d -o CMakeFiles/nebu_input.dir/system_keynames.c.o -c /home/mateus/gltron/nebu/input/system_keynames.c

nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nebu_input.dir/system_keynames.c.i"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/nebu/input/system_keynames.c > CMakeFiles/nebu_input.dir/system_keynames.c.i

nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nebu_input.dir/system_keynames.c.s"
	cd /home/mateus/gltron/nebu/input && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/nebu/input/system_keynames.c -o CMakeFiles/nebu_input.dir/system_keynames.c.s

# Object files for target nebu_input
nebu_input_OBJECTS = \
"CMakeFiles/nebu_input.dir/custom_keys.c.o" \
"CMakeFiles/nebu_input.dir/input_system.c.o" \
"CMakeFiles/nebu_input.dir/system_keynames.c.o"

# External object files for target nebu_input
nebu_input_EXTERNAL_OBJECTS =

nebu/input/libnebu_input.a: nebu/input/CMakeFiles/nebu_input.dir/custom_keys.c.o
nebu/input/libnebu_input.a: nebu/input/CMakeFiles/nebu_input.dir/input_system.c.o
nebu/input/libnebu_input.a: nebu/input/CMakeFiles/nebu_input.dir/system_keynames.c.o
nebu/input/libnebu_input.a: nebu/input/CMakeFiles/nebu_input.dir/build.make
nebu/input/libnebu_input.a: nebu/input/CMakeFiles/nebu_input.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mateus/gltron/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C static library libnebu_input.a"
	cd /home/mateus/gltron/nebu/input && $(CMAKE_COMMAND) -P CMakeFiles/nebu_input.dir/cmake_clean_target.cmake
	cd /home/mateus/gltron/nebu/input && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nebu_input.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
nebu/input/CMakeFiles/nebu_input.dir/build: nebu/input/libnebu_input.a
.PHONY : nebu/input/CMakeFiles/nebu_input.dir/build

nebu/input/CMakeFiles/nebu_input.dir/clean:
	cd /home/mateus/gltron/nebu/input && $(CMAKE_COMMAND) -P CMakeFiles/nebu_input.dir/cmake_clean.cmake
.PHONY : nebu/input/CMakeFiles/nebu_input.dir/clean

nebu/input/CMakeFiles/nebu_input.dir/depend:
	cd /home/mateus/gltron && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mateus/gltron /home/mateus/gltron/nebu/input /home/mateus/gltron /home/mateus/gltron/nebu/input /home/mateus/gltron/nebu/input/CMakeFiles/nebu_input.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : nebu/input/CMakeFiles/nebu_input.dir/depend

