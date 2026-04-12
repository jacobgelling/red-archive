cmake_minimum_required(VERSION 3.24)

project(red-archive LANGUAGES C VERSION 1.0.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

configure_file(${PROJECT_SOURCE_DIR}/include/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/version.h)

add_executable(red-archive)
target_sources(red-archive PRIVATE ${PROJECT_SOURCE_DIR}/src/cli.c ${PROJECT_SOURCE_DIR}/src/archive.c)
target_include_directories(red-archive PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

if (WIN32)
    target_compile_definitions(red-archive PRIVATE _CRT_SECURE_NO_WARNINGS)
    include(FetchContent)
    FetchContent_Declare(
        dirent
        URL https://github.com/tronkko/dirent/archive/refs/tags/1.26.zip
        URL_HASH SHA256=6FFCC318F00BE192ACB611C58AA58EE0CFD96776010680D7F38CF24F3DD8BAF9
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(dirent)
    target_link_libraries(red-archive PRIVATE dirent)
    target_include_directories(red-archive PRIVATE ${dirent_SOURCE_DIR}/include)
endif ()

include(GNUInstallDirs)
install(TARGETS red-archive RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
