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
CMAKE_BINARY_DIR = /home/mateus/gltron/build

# Include any dependencies generated for this target.
include lua5/CMakeFiles/lualib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include lua5/CMakeFiles/lualib.dir/compiler_depend.make

# Include the progress variables for this target.
include lua5/CMakeFiles/lualib.dir/progress.make

# Include the compile flags for this target's objects.
include lua5/CMakeFiles/lualib.dir/flags.make

lua5/CMakeFiles/lualib.dir/lapi.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lapi.c.o: ../lua5/lapi.c
lua5/CMakeFiles/lualib.dir/lapi.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object lua5/CMakeFiles/lualib.dir/lapi.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lapi.c.o -MF CMakeFiles/lualib.dir/lapi.c.o.d -o CMakeFiles/lualib.dir/lapi.c.o -c /home/mateus/gltron/lua5/lapi.c

lua5/CMakeFiles/lualib.dir/lapi.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lapi.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lapi.c > CMakeFiles/lualib.dir/lapi.c.i

lua5/CMakeFiles/lualib.dir/lapi.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lapi.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lapi.c -o CMakeFiles/lualib.dir/lapi.c.s

lua5/CMakeFiles/lualib.dir/lcode.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lcode.c.o: ../lua5/lcode.c
lua5/CMakeFiles/lualib.dir/lcode.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object lua5/CMakeFiles/lualib.dir/lcode.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lcode.c.o -MF CMakeFiles/lualib.dir/lcode.c.o.d -o CMakeFiles/lualib.dir/lcode.c.o -c /home/mateus/gltron/lua5/lcode.c

lua5/CMakeFiles/lualib.dir/lcode.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lcode.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lcode.c > CMakeFiles/lualib.dir/lcode.c.i

lua5/CMakeFiles/lualib.dir/lcode.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lcode.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lcode.c -o CMakeFiles/lualib.dir/lcode.c.s

lua5/CMakeFiles/lualib.dir/ldebug.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/ldebug.c.o: ../lua5/ldebug.c
lua5/CMakeFiles/lualib.dir/ldebug.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object lua5/CMakeFiles/lualib.dir/ldebug.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/ldebug.c.o -MF CMakeFiles/lualib.dir/ldebug.c.o.d -o CMakeFiles/lualib.dir/ldebug.c.o -c /home/mateus/gltron/lua5/ldebug.c

lua5/CMakeFiles/lualib.dir/ldebug.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/ldebug.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/ldebug.c > CMakeFiles/lualib.dir/ldebug.c.i

lua5/CMakeFiles/lualib.dir/ldebug.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/ldebug.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/ldebug.c -o CMakeFiles/lualib.dir/ldebug.c.s

lua5/CMakeFiles/lualib.dir/ldo.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/ldo.c.o: ../lua5/ldo.c
lua5/CMakeFiles/lualib.dir/ldo.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object lua5/CMakeFiles/lualib.dir/ldo.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/ldo.c.o -MF CMakeFiles/lualib.dir/ldo.c.o.d -o CMakeFiles/lualib.dir/ldo.c.o -c /home/mateus/gltron/lua5/ldo.c

lua5/CMakeFiles/lualib.dir/ldo.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/ldo.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/ldo.c > CMakeFiles/lualib.dir/ldo.c.i

lua5/CMakeFiles/lualib.dir/ldo.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/ldo.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/ldo.c -o CMakeFiles/lualib.dir/ldo.c.s

lua5/CMakeFiles/lualib.dir/ldump.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/ldump.c.o: ../lua5/ldump.c
lua5/CMakeFiles/lualib.dir/ldump.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object lua5/CMakeFiles/lualib.dir/ldump.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/ldump.c.o -MF CMakeFiles/lualib.dir/ldump.c.o.d -o CMakeFiles/lualib.dir/ldump.c.o -c /home/mateus/gltron/lua5/ldump.c

lua5/CMakeFiles/lualib.dir/ldump.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/ldump.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/ldump.c > CMakeFiles/lualib.dir/ldump.c.i

lua5/CMakeFiles/lualib.dir/ldump.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/ldump.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/ldump.c -o CMakeFiles/lualib.dir/ldump.c.s

lua5/CMakeFiles/lualib.dir/lfunc.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lfunc.c.o: ../lua5/lfunc.c
lua5/CMakeFiles/lualib.dir/lfunc.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object lua5/CMakeFiles/lualib.dir/lfunc.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lfunc.c.o -MF CMakeFiles/lualib.dir/lfunc.c.o.d -o CMakeFiles/lualib.dir/lfunc.c.o -c /home/mateus/gltron/lua5/lfunc.c

lua5/CMakeFiles/lualib.dir/lfunc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lfunc.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lfunc.c > CMakeFiles/lualib.dir/lfunc.c.i

lua5/CMakeFiles/lualib.dir/lfunc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lfunc.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lfunc.c -o CMakeFiles/lualib.dir/lfunc.c.s

lua5/CMakeFiles/lualib.dir/lgc.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lgc.c.o: ../lua5/lgc.c
lua5/CMakeFiles/lualib.dir/lgc.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object lua5/CMakeFiles/lualib.dir/lgc.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lgc.c.o -MF CMakeFiles/lualib.dir/lgc.c.o.d -o CMakeFiles/lualib.dir/lgc.c.o -c /home/mateus/gltron/lua5/lgc.c

lua5/CMakeFiles/lualib.dir/lgc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lgc.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lgc.c > CMakeFiles/lualib.dir/lgc.c.i

lua5/CMakeFiles/lualib.dir/lgc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lgc.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lgc.c -o CMakeFiles/lualib.dir/lgc.c.s

lua5/CMakeFiles/lualib.dir/llex.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/llex.c.o: ../lua5/llex.c
lua5/CMakeFiles/lualib.dir/llex.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object lua5/CMakeFiles/lualib.dir/llex.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/llex.c.o -MF CMakeFiles/lualib.dir/llex.c.o.d -o CMakeFiles/lualib.dir/llex.c.o -c /home/mateus/gltron/lua5/llex.c

lua5/CMakeFiles/lualib.dir/llex.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/llex.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/llex.c > CMakeFiles/lualib.dir/llex.c.i

lua5/CMakeFiles/lualib.dir/llex.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/llex.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/llex.c -o CMakeFiles/lualib.dir/llex.c.s

lua5/CMakeFiles/lualib.dir/lmem.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lmem.c.o: ../lua5/lmem.c
lua5/CMakeFiles/lualib.dir/lmem.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object lua5/CMakeFiles/lualib.dir/lmem.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lmem.c.o -MF CMakeFiles/lualib.dir/lmem.c.o.d -o CMakeFiles/lualib.dir/lmem.c.o -c /home/mateus/gltron/lua5/lmem.c

lua5/CMakeFiles/lualib.dir/lmem.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lmem.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lmem.c > CMakeFiles/lualib.dir/lmem.c.i

lua5/CMakeFiles/lualib.dir/lmem.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lmem.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lmem.c -o CMakeFiles/lualib.dir/lmem.c.s

lua5/CMakeFiles/lualib.dir/lobject.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lobject.c.o: ../lua5/lobject.c
lua5/CMakeFiles/lualib.dir/lobject.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building C object lua5/CMakeFiles/lualib.dir/lobject.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lobject.c.o -MF CMakeFiles/lualib.dir/lobject.c.o.d -o CMakeFiles/lualib.dir/lobject.c.o -c /home/mateus/gltron/lua5/lobject.c

lua5/CMakeFiles/lualib.dir/lobject.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lobject.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lobject.c > CMakeFiles/lualib.dir/lobject.c.i

lua5/CMakeFiles/lualib.dir/lobject.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lobject.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lobject.c -o CMakeFiles/lualib.dir/lobject.c.s

lua5/CMakeFiles/lualib.dir/lopcodes.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lopcodes.c.o: ../lua5/lopcodes.c
lua5/CMakeFiles/lualib.dir/lopcodes.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building C object lua5/CMakeFiles/lualib.dir/lopcodes.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lopcodes.c.o -MF CMakeFiles/lualib.dir/lopcodes.c.o.d -o CMakeFiles/lualib.dir/lopcodes.c.o -c /home/mateus/gltron/lua5/lopcodes.c

lua5/CMakeFiles/lualib.dir/lopcodes.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lopcodes.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lopcodes.c > CMakeFiles/lualib.dir/lopcodes.c.i

lua5/CMakeFiles/lualib.dir/lopcodes.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lopcodes.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lopcodes.c -o CMakeFiles/lualib.dir/lopcodes.c.s

lua5/CMakeFiles/lualib.dir/lparser.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lparser.c.o: ../lua5/lparser.c
lua5/CMakeFiles/lualib.dir/lparser.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building C object lua5/CMakeFiles/lualib.dir/lparser.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lparser.c.o -MF CMakeFiles/lualib.dir/lparser.c.o.d -o CMakeFiles/lualib.dir/lparser.c.o -c /home/mateus/gltron/lua5/lparser.c

lua5/CMakeFiles/lualib.dir/lparser.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lparser.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lparser.c > CMakeFiles/lualib.dir/lparser.c.i

lua5/CMakeFiles/lualib.dir/lparser.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lparser.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lparser.c -o CMakeFiles/lualib.dir/lparser.c.s

lua5/CMakeFiles/lualib.dir/lstate.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lstate.c.o: ../lua5/lstate.c
lua5/CMakeFiles/lualib.dir/lstate.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building C object lua5/CMakeFiles/lualib.dir/lstate.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lstate.c.o -MF CMakeFiles/lualib.dir/lstate.c.o.d -o CMakeFiles/lualib.dir/lstate.c.o -c /home/mateus/gltron/lua5/lstate.c

lua5/CMakeFiles/lualib.dir/lstate.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lstate.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lstate.c > CMakeFiles/lualib.dir/lstate.c.i

lua5/CMakeFiles/lualib.dir/lstate.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lstate.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lstate.c -o CMakeFiles/lualib.dir/lstate.c.s

lua5/CMakeFiles/lualib.dir/lstring.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lstring.c.o: ../lua5/lstring.c
lua5/CMakeFiles/lualib.dir/lstring.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building C object lua5/CMakeFiles/lualib.dir/lstring.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lstring.c.o -MF CMakeFiles/lualib.dir/lstring.c.o.d -o CMakeFiles/lualib.dir/lstring.c.o -c /home/mateus/gltron/lua5/lstring.c

lua5/CMakeFiles/lualib.dir/lstring.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lstring.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lstring.c > CMakeFiles/lualib.dir/lstring.c.i

lua5/CMakeFiles/lualib.dir/lstring.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lstring.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lstring.c -o CMakeFiles/lualib.dir/lstring.c.s

lua5/CMakeFiles/lualib.dir/ltable.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/ltable.c.o: ../lua5/ltable.c
lua5/CMakeFiles/lualib.dir/ltable.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building C object lua5/CMakeFiles/lualib.dir/ltable.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/ltable.c.o -MF CMakeFiles/lualib.dir/ltable.c.o.d -o CMakeFiles/lualib.dir/ltable.c.o -c /home/mateus/gltron/lua5/ltable.c

lua5/CMakeFiles/lualib.dir/ltable.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/ltable.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/ltable.c > CMakeFiles/lualib.dir/ltable.c.i

lua5/CMakeFiles/lualib.dir/ltable.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/ltable.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/ltable.c -o CMakeFiles/lualib.dir/ltable.c.s

lua5/CMakeFiles/lualib.dir/ltests.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/ltests.c.o: ../lua5/ltests.c
lua5/CMakeFiles/lualib.dir/ltests.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building C object lua5/CMakeFiles/lualib.dir/ltests.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/ltests.c.o -MF CMakeFiles/lualib.dir/ltests.c.o.d -o CMakeFiles/lualib.dir/ltests.c.o -c /home/mateus/gltron/lua5/ltests.c

lua5/CMakeFiles/lualib.dir/ltests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/ltests.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/ltests.c > CMakeFiles/lualib.dir/ltests.c.i

lua5/CMakeFiles/lualib.dir/ltests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/ltests.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/ltests.c -o CMakeFiles/lualib.dir/ltests.c.s

lua5/CMakeFiles/lualib.dir/ltm.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/ltm.c.o: ../lua5/ltm.c
lua5/CMakeFiles/lualib.dir/ltm.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Building C object lua5/CMakeFiles/lualib.dir/ltm.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/ltm.c.o -MF CMakeFiles/lualib.dir/ltm.c.o.d -o CMakeFiles/lualib.dir/ltm.c.o -c /home/mateus/gltron/lua5/ltm.c

lua5/CMakeFiles/lualib.dir/ltm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/ltm.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/ltm.c > CMakeFiles/lualib.dir/ltm.c.i

lua5/CMakeFiles/lualib.dir/ltm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/ltm.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/ltm.c -o CMakeFiles/lualib.dir/ltm.c.s

lua5/CMakeFiles/lualib.dir/lundump.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lundump.c.o: ../lua5/lundump.c
lua5/CMakeFiles/lualib.dir/lundump.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_18) "Building C object lua5/CMakeFiles/lualib.dir/lundump.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lundump.c.o -MF CMakeFiles/lualib.dir/lundump.c.o.d -o CMakeFiles/lualib.dir/lundump.c.o -c /home/mateus/gltron/lua5/lundump.c

lua5/CMakeFiles/lualib.dir/lundump.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lundump.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lundump.c > CMakeFiles/lualib.dir/lundump.c.i

lua5/CMakeFiles/lualib.dir/lundump.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lundump.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lundump.c -o CMakeFiles/lualib.dir/lundump.c.s

lua5/CMakeFiles/lualib.dir/lvm.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lvm.c.o: ../lua5/lvm.c
lua5/CMakeFiles/lualib.dir/lvm.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_19) "Building C object lua5/CMakeFiles/lualib.dir/lvm.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lvm.c.o -MF CMakeFiles/lualib.dir/lvm.c.o.d -o CMakeFiles/lualib.dir/lvm.c.o -c /home/mateus/gltron/lua5/lvm.c

lua5/CMakeFiles/lualib.dir/lvm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lvm.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lvm.c > CMakeFiles/lualib.dir/lvm.c.i

lua5/CMakeFiles/lualib.dir/lvm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lvm.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lvm.c -o CMakeFiles/lualib.dir/lvm.c.s

lua5/CMakeFiles/lualib.dir/lzio.c.o: lua5/CMakeFiles/lualib.dir/flags.make
lua5/CMakeFiles/lualib.dir/lzio.c.o: ../lua5/lzio.c
lua5/CMakeFiles/lualib.dir/lzio.c.o: lua5/CMakeFiles/lualib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_20) "Building C object lua5/CMakeFiles/lualib.dir/lzio.c.o"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lua5/CMakeFiles/lualib.dir/lzio.c.o -MF CMakeFiles/lualib.dir/lzio.c.o.d -o CMakeFiles/lualib.dir/lzio.c.o -c /home/mateus/gltron/lua5/lzio.c

lua5/CMakeFiles/lualib.dir/lzio.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lualib.dir/lzio.c.i"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mateus/gltron/lua5/lzio.c > CMakeFiles/lualib.dir/lzio.c.i

lua5/CMakeFiles/lualib.dir/lzio.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lualib.dir/lzio.c.s"
	cd /home/mateus/gltron/build/lua5 && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mateus/gltron/lua5/lzio.c -o CMakeFiles/lualib.dir/lzio.c.s

# Object files for target lualib
lualib_OBJECTS = \
"CMakeFiles/lualib.dir/lapi.c.o" \
"CMakeFiles/lualib.dir/lcode.c.o" \
"CMakeFiles/lualib.dir/ldebug.c.o" \
"CMakeFiles/lualib.dir/ldo.c.o" \
"CMakeFiles/lualib.dir/ldump.c.o" \
"CMakeFiles/lualib.dir/lfunc.c.o" \
"CMakeFiles/lualib.dir/lgc.c.o" \
"CMakeFiles/lualib.dir/llex.c.o" \
"CMakeFiles/lualib.dir/lmem.c.o" \
"CMakeFiles/lualib.dir/lobject.c.o" \
"CMakeFiles/lualib.dir/lopcodes.c.o" \
"CMakeFiles/lualib.dir/lparser.c.o" \
"CMakeFiles/lualib.dir/lstate.c.o" \
"CMakeFiles/lualib.dir/lstring.c.o" \
"CMakeFiles/lualib.dir/ltable.c.o" \
"CMakeFiles/lualib.dir/ltests.c.o" \
"CMakeFiles/lualib.dir/ltm.c.o" \
"CMakeFiles/lualib.dir/lundump.c.o" \
"CMakeFiles/lualib.dir/lvm.c.o" \
"CMakeFiles/lualib.dir/lzio.c.o"

# External object files for target lualib
lualib_EXTERNAL_OBJECTS =

lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lapi.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lcode.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/ldebug.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/ldo.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/ldump.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lfunc.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lgc.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/llex.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lmem.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lobject.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lopcodes.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lparser.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lstate.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lstring.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/ltable.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/ltests.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/ltm.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lundump.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lvm.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/lzio.c.o
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/build.make
lua5/liblualib.a: lua5/CMakeFiles/lualib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mateus/gltron/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_21) "Linking C static library liblualib.a"
	cd /home/mateus/gltron/build/lua5 && $(CMAKE_COMMAND) -P CMakeFiles/lualib.dir/cmake_clean_target.cmake
	cd /home/mateus/gltron/build/lua5 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lualib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lua5/CMakeFiles/lualib.dir/build: lua5/liblualib.a
.PHONY : lua5/CMakeFiles/lualib.dir/build

lua5/CMakeFiles/lualib.dir/clean:
	cd /home/mateus/gltron/build/lua5 && $(CMAKE_COMMAND) -P CMakeFiles/lualib.dir/cmake_clean.cmake
.PHONY : lua5/CMakeFiles/lualib.dir/clean

lua5/CMakeFiles/lualib.dir/depend:
	cd /home/mateus/gltron/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mateus/gltron /home/mateus/gltron/lua5 /home/mateus/gltron/build /home/mateus/gltron/build/lua5 /home/mateus/gltron/build/lua5/CMakeFiles/lualib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lua5/CMakeFiles/lualib.dir/depend

