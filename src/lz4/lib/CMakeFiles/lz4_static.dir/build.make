# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/tesrasupernet/src/lz4/contrib/cmake_unofficial

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/tesrasupernet/src/lz4/lib

# Include any dependencies generated for this target.
include CMakeFiles/lz4_static.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lz4_static.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lz4_static.dir/flags.make

CMakeFiles/lz4_static.dir/lz4.c.o: CMakeFiles/lz4_static.dir/flags.make
CMakeFiles/lz4_static.dir/lz4.c.o: lz4.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/tesrasupernet/src/lz4/lib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/lz4_static.dir/lz4.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lz4_static.dir/lz4.c.o   -c /root/tesrasupernet/src/lz4/lib/lz4.c

CMakeFiles/lz4_static.dir/lz4.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lz4_static.dir/lz4.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /root/tesrasupernet/src/lz4/lib/lz4.c > CMakeFiles/lz4_static.dir/lz4.c.i

CMakeFiles/lz4_static.dir/lz4.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lz4_static.dir/lz4.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /root/tesrasupernet/src/lz4/lib/lz4.c -o CMakeFiles/lz4_static.dir/lz4.c.s

CMakeFiles/lz4_static.dir/lz4.c.o.requires:

.PHONY : CMakeFiles/lz4_static.dir/lz4.c.o.requires

CMakeFiles/lz4_static.dir/lz4.c.o.provides: CMakeFiles/lz4_static.dir/lz4.c.o.requires
	$(MAKE) -f CMakeFiles/lz4_static.dir/build.make CMakeFiles/lz4_static.dir/lz4.c.o.provides.build
.PHONY : CMakeFiles/lz4_static.dir/lz4.c.o.provides

CMakeFiles/lz4_static.dir/lz4.c.o.provides.build: CMakeFiles/lz4_static.dir/lz4.c.o


CMakeFiles/lz4_static.dir/lz4hc.c.o: CMakeFiles/lz4_static.dir/flags.make
CMakeFiles/lz4_static.dir/lz4hc.c.o: lz4hc.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/tesrasupernet/src/lz4/lib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/lz4_static.dir/lz4hc.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lz4_static.dir/lz4hc.c.o   -c /root/tesrasupernet/src/lz4/lib/lz4hc.c

CMakeFiles/lz4_static.dir/lz4hc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lz4_static.dir/lz4hc.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /root/tesrasupernet/src/lz4/lib/lz4hc.c > CMakeFiles/lz4_static.dir/lz4hc.c.i

CMakeFiles/lz4_static.dir/lz4hc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lz4_static.dir/lz4hc.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /root/tesrasupernet/src/lz4/lib/lz4hc.c -o CMakeFiles/lz4_static.dir/lz4hc.c.s

CMakeFiles/lz4_static.dir/lz4hc.c.o.requires:

.PHONY : CMakeFiles/lz4_static.dir/lz4hc.c.o.requires

CMakeFiles/lz4_static.dir/lz4hc.c.o.provides: CMakeFiles/lz4_static.dir/lz4hc.c.o.requires
	$(MAKE) -f CMakeFiles/lz4_static.dir/build.make CMakeFiles/lz4_static.dir/lz4hc.c.o.provides.build
.PHONY : CMakeFiles/lz4_static.dir/lz4hc.c.o.provides

CMakeFiles/lz4_static.dir/lz4hc.c.o.provides.build: CMakeFiles/lz4_static.dir/lz4hc.c.o


CMakeFiles/lz4_static.dir/lz4frame.c.o: CMakeFiles/lz4_static.dir/flags.make
CMakeFiles/lz4_static.dir/lz4frame.c.o: lz4frame.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/tesrasupernet/src/lz4/lib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/lz4_static.dir/lz4frame.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lz4_static.dir/lz4frame.c.o   -c /root/tesrasupernet/src/lz4/lib/lz4frame.c

CMakeFiles/lz4_static.dir/lz4frame.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lz4_static.dir/lz4frame.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /root/tesrasupernet/src/lz4/lib/lz4frame.c > CMakeFiles/lz4_static.dir/lz4frame.c.i

CMakeFiles/lz4_static.dir/lz4frame.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lz4_static.dir/lz4frame.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /root/tesrasupernet/src/lz4/lib/lz4frame.c -o CMakeFiles/lz4_static.dir/lz4frame.c.s

CMakeFiles/lz4_static.dir/lz4frame.c.o.requires:

.PHONY : CMakeFiles/lz4_static.dir/lz4frame.c.o.requires

CMakeFiles/lz4_static.dir/lz4frame.c.o.provides: CMakeFiles/lz4_static.dir/lz4frame.c.o.requires
	$(MAKE) -f CMakeFiles/lz4_static.dir/build.make CMakeFiles/lz4_static.dir/lz4frame.c.o.provides.build
.PHONY : CMakeFiles/lz4_static.dir/lz4frame.c.o.provides

CMakeFiles/lz4_static.dir/lz4frame.c.o.provides.build: CMakeFiles/lz4_static.dir/lz4frame.c.o


CMakeFiles/lz4_static.dir/xxhash.c.o: CMakeFiles/lz4_static.dir/flags.make
CMakeFiles/lz4_static.dir/xxhash.c.o: xxhash.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/tesrasupernet/src/lz4/lib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/lz4_static.dir/xxhash.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lz4_static.dir/xxhash.c.o   -c /root/tesrasupernet/src/lz4/lib/xxhash.c

CMakeFiles/lz4_static.dir/xxhash.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lz4_static.dir/xxhash.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /root/tesrasupernet/src/lz4/lib/xxhash.c > CMakeFiles/lz4_static.dir/xxhash.c.i

CMakeFiles/lz4_static.dir/xxhash.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lz4_static.dir/xxhash.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /root/tesrasupernet/src/lz4/lib/xxhash.c -o CMakeFiles/lz4_static.dir/xxhash.c.s

CMakeFiles/lz4_static.dir/xxhash.c.o.requires:

.PHONY : CMakeFiles/lz4_static.dir/xxhash.c.o.requires

CMakeFiles/lz4_static.dir/xxhash.c.o.provides: CMakeFiles/lz4_static.dir/xxhash.c.o.requires
	$(MAKE) -f CMakeFiles/lz4_static.dir/build.make CMakeFiles/lz4_static.dir/xxhash.c.o.provides.build
.PHONY : CMakeFiles/lz4_static.dir/xxhash.c.o.provides

CMakeFiles/lz4_static.dir/xxhash.c.o.provides.build: CMakeFiles/lz4_static.dir/xxhash.c.o


# Object files for target lz4_static
lz4_static_OBJECTS = \
"CMakeFiles/lz4_static.dir/lz4.c.o" \
"CMakeFiles/lz4_static.dir/lz4hc.c.o" \
"CMakeFiles/lz4_static.dir/lz4frame.c.o" \
"CMakeFiles/lz4_static.dir/xxhash.c.o"

# External object files for target lz4_static
lz4_static_EXTERNAL_OBJECTS =

liblz4.a: CMakeFiles/lz4_static.dir/lz4.c.o
liblz4.a: CMakeFiles/lz4_static.dir/lz4hc.c.o
liblz4.a: CMakeFiles/lz4_static.dir/lz4frame.c.o
liblz4.a: CMakeFiles/lz4_static.dir/xxhash.c.o
liblz4.a: CMakeFiles/lz4_static.dir/build.make
liblz4.a: CMakeFiles/lz4_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/tesrasupernet/src/lz4/lib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C static library liblz4.a"
	$(CMAKE_COMMAND) -P CMakeFiles/lz4_static.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lz4_static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lz4_static.dir/build: liblz4.a

.PHONY : CMakeFiles/lz4_static.dir/build

CMakeFiles/lz4_static.dir/requires: CMakeFiles/lz4_static.dir/lz4.c.o.requires
CMakeFiles/lz4_static.dir/requires: CMakeFiles/lz4_static.dir/lz4hc.c.o.requires
CMakeFiles/lz4_static.dir/requires: CMakeFiles/lz4_static.dir/lz4frame.c.o.requires
CMakeFiles/lz4_static.dir/requires: CMakeFiles/lz4_static.dir/xxhash.c.o.requires

.PHONY : CMakeFiles/lz4_static.dir/requires

CMakeFiles/lz4_static.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lz4_static.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lz4_static.dir/clean

CMakeFiles/lz4_static.dir/depend:
	cd /root/tesrasupernet/src/lz4/lib && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/tesrasupernet/src/lz4/contrib/cmake_unofficial /root/tesrasupernet/src/lz4/contrib/cmake_unofficial /root/tesrasupernet/src/lz4/lib /root/tesrasupernet/src/lz4/lib /root/tesrasupernet/src/lz4/lib/CMakeFiles/lz4_static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lz4_static.dir/depend

