cmake_minimum_required(VERSION 3.24)

project(red-archive LANGUAGES C VERSION 1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(red-archive ${PROJECT_SOURCE_DIR}/src/cli.c ${PROJECT_SOURCE_DIR}/src/archive.c)

target_include_directories(red-archive PRIVATE ${PROJECT_SOURCE_DIR}/include)

if (WIN32)
    target_compile_definitions(red-archive PRIVATE _CRT_SECURE_NO_WARNINGS)
    include(FetchContent)
    FetchContent_Declare(
            dirent
            URL https://github.com/tronkko/dirent/archive/refs/tags/1.26.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(dirent)
    target_link_libraries(red-archive PRIVATE dirent)
    target_include_directories(red-archive PRIVATE ${dirent_SOURCE_DIR}/include)
endif ()
