# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build

# Include any dependencies generated for this target.
include hello-exe/CMakeFiles/Tutorial.dir/depend.make

# Include the progress variables for this target.
include hello-exe/CMakeFiles/Tutorial.dir/progress.make

# Include the compile flags for this target's objects.
include hello-exe/CMakeFiles/Tutorial.dir/flags.make

hello-exe/CMakeFiles/Tutorial.dir/main.cpp.o: hello-exe/CMakeFiles/Tutorial.dir/flags.make
hello-exe/CMakeFiles/Tutorial.dir/main.cpp.o: ../hello-exe/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object hello-exe/CMakeFiles/Tutorial.dir/main.cpp.o"
	cd /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/hello-exe && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Tutorial.dir/main.cpp.o -c /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/hello-exe/main.cpp

hello-exe/CMakeFiles/Tutorial.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Tutorial.dir/main.cpp.i"
	cd /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/hello-exe && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/hello-exe/main.cpp > CMakeFiles/Tutorial.dir/main.cpp.i

hello-exe/CMakeFiles/Tutorial.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Tutorial.dir/main.cpp.s"
	cd /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/hello-exe && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/hello-exe/main.cpp -o CMakeFiles/Tutorial.dir/main.cpp.s

# Object files for target Tutorial
Tutorial_OBJECTS = \
"CMakeFiles/Tutorial.dir/main.cpp.o"

# External object files for target Tutorial
Tutorial_EXTERNAL_OBJECTS =

hello-exe/Tutorial: hello-exe/CMakeFiles/Tutorial.dir/main.cpp.o
hello-exe/Tutorial: hello-exe/CMakeFiles/Tutorial.dir/build.make
hello-exe/Tutorial: say-hello/libsay-hello.a
hello-exe/Tutorial: hello-exe/CMakeFiles/Tutorial.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Tutorial"
	cd /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/hello-exe && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Tutorial.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
hello-exe/CMakeFiles/Tutorial.dir/build: hello-exe/Tutorial

.PHONY : hello-exe/CMakeFiles/Tutorial.dir/build

hello-exe/CMakeFiles/Tutorial.dir/clean:
	cd /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/hello-exe && $(CMAKE_COMMAND) -P CMakeFiles/Tutorial.dir/cmake_clean.cmake
.PHONY : hello-exe/CMakeFiles/Tutorial.dir/clean

hello-exe/CMakeFiles/Tutorial.dir/depend:
	cd /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/hello-exe /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/hello-exe /mnt/c/Users/kubec/Documents/CMakePractice/Tutorial/build/hello-exe/CMakeFiles/Tutorial.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : hello-exe/CMakeFiles/Tutorial.dir/depend

