cmake_minimum_required(VERSION 3.17)

project(ChorusKit VERSION 0.0.0.1 LANGUAGES CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(CHORUSKIT_BUILD_TRANSLATIONS "Build translations" ON)
option(CHORUSKIT_BUILD_TESTS "Build test cases" ON)
option(CHORUSKIT_BUILD_DOCUMENTATIONS "Build documentations" OFF)
option(CHORUSKIT_INSTALL "Install library" ON)
option(CHORUSKIT_VCPKG_TOOLS_HINT "Install executables to tools directory" OFF)

# ----------------------------------
# CMake Settings
# ----------------------------------
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
set(CHORUSKIT_VERSION ${PROJECT_VERSION})
set(CHORUSKIT_INSTALL_NAME ${PROJECT_NAME})
set(CHORUSKIT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CHORUSKIT_TRANSLATIONS zh_CN zh_TW ja_JP)

# ----------------------------------
# Find basic dependencies
# ----------------------------------
find_package(qmsetup REQUIRED)

qm_import(Filesystem)
qm_init_directories()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(src)

add_subdirectory(share)