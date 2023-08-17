project(qBreakpad VERSION 0.2.1.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(unofficial-breakpad CONFIG REQUIRED)

ck_add_qt_module(_qt_libs Core Network)

include(../config.cmake)

file(GLOB_RECURSE _src *.h *.cpp)

# This library is released under LGPL 3.0 license,
# We static link it for convenience, so that ChorusKit is released under LGPL 3.0 too,
choruskit_add_library(${PROJECT_NAME} STATIC ${_src})

target_compile_definitions(${PROJECT_NAME} PUBLIC QBREAKPAD_STATIC)

target_link_libraries(${PROJECT_NAME} PRIVATE
    unofficial::breakpad::libbreakpad
    unofficial::breakpad::libbreakpad_client
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${_qt_libs}
)

choruskit_include_directories(${PROJECT_NAME} .)

choruskit_install_headers(.)