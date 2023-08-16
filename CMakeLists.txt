cmake_minimum_required(VERSION 3.17)

project(ChorusKit VERSION 0.0.0.1 LANGUAGES CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(CHORUSKIT_BUILD_TRANSLATIONS "Build translations" ON)
option(CHORUSKIT_BUILD_TESTS "Build test cases" ON)
option(CHORUSKIT_INSTALL "Install library" ON)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

if(CHORUSKIT_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(CK_CMAKE_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CHORUSKIT_VERSION ${PROJECT_VERSION})
set(CHORUSKIT_INSTALL_NAME ${PROJECT_NAME})
set(CHORUSKIT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CHORUSKIT_TRANSLATIONS zh_CN zh_HK ja_JP)

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(src)

add_subdirectory(share)