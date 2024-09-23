# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(CHIP8Emulator VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find and link required libraries
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Add source files
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Check if source files were found, and if not, throw an error
if(NOT SOURCES)
    message(FATAL_ERROR "No source files found. Check your src/ directory.")
endif()

# Add the executable for your project
add_executable(chip8-emulator ${SOURCES})

# Include directories for your header files (e.g., src/ and external libs)
target_include_directories(chip8-emulator PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

# Link the libraries
target_link_libraries(chip8-emulator PRIVATE OpenGL::GL GLUT::GLUT)

# Add compiler warnings (optional)
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(chip8-emulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

