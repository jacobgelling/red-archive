# Red Archive
# MIT License
# Copyright (c) 2020 Jacob Gelling

# Set minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Set project name, language and version
project(red-archive LANGUAGES C VERSION 0.1)

# Create version header file
configure_file(${PROJECT_SOURCE_DIR}/include/version.h.in ${PROJECT_SOURCE_DIR}/include/version.h)

# Include dirent for Windows
if (WIN32)
    include_directories(${PROJECT_SOURCE_DIR}/dirent)
endif (WIN32)

# Include header files
include_directories(${PROJECT_SOURCE_DIR}/include)

# Set output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Set executables to compile
add_executable(red-archive ${PROJECT_SOURCE_DIR}/src/cli.c ${PROJECT_SOURCE_DIR}/src/archive.c)
