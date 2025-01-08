# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(CHIP8Emulator VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add source files
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Check if source files were found, and if not, throw an error
if(NOT SOURCES)
    message(FATAL_ERROR "No source files found. Check your src/ directory.")
endif()

# Add an option to compile for WebAssembly
option(COMPILE_TO_WASM "Compile to WebAssembly using Emscripten" OFF)

# Automatically set build directories
if(COMPILE_TO_WASM)
    set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build-web")
else()
    set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")
endif()

# Ensure that builds are placed in the designated directory
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please use a build directory like ${BUILD_DIR}.")
endif()

# WASM-specific settings
if(COMPILE_TO_WASM)
    set(CMAKE_SYSTEM_NAME WebAssembly)
    set(CMAKE_C_COMPILER emcc)
    set(CMAKE_CXX_COMPILER em++)

    # Enable Legacy GL Emulation for WebAssembly
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s LEGACY_GL_EMULATION=1 -s USE_GL=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=2")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s MODULARIZE=1 -s EXPORT_ES6=1 -s STACK_SIZE=65536")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s LEGACY_GL_EMULATION=1 -s GL_DEBUG=1 -s USE_GL=1")

    # Override the linker command to remove unsupported flags
    set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

    # Add the executable for WebAssembly
    add_executable(chip8-emulator ${SOURCES})

    # Include directories for your header files
    target_include_directories(chip8-emulator PRIVATE ${CMAKE_SOURCE_DIR}/include)

    # Set the output file explicitly
    set_target_properties(chip8-emulator PROPERTIES OUTPUT_NAME "chip8")
else()
    # Native build (OpenGL and GLUT)
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)

    # Add the executable for your native build
    add_executable(chip8-emulator ${SOURCES})

    # Include directories for your header files
    target_include_directories(chip8-emulator PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})

    # Link the libraries
    target_link_libraries(chip8-emulator PRIVATE OpenGL::GL GLUT::GLUT)

    # Add compiler warnings (optional)
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(chip8-emulator PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
