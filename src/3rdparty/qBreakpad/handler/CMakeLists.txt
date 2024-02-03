project(qBreakpad VERSION 0.2.1.1 LANGUAGES CXX)

find_package(unofficial-breakpad CONFIG REQUIRED)

include(../config.cmake)

file(GLOB_RECURSE _src *.h *.cpp)

# This library is released under LGPL 3.0 license,
# We static link it for convenience, so that ChorusKit is released under LGPL 3.0 too,
choruskit_add_library(${PROJECT_NAME} STATIC AUTOGEN
    SOURCES ${_src}
    LINKS_PRIVATE unofficial::breakpad::libbreakpad unofficial::breakpad::libbreakpad_client
    QT_LINKS Core Network
    SYNC_INCLUDE_PREFIX ${PROJECT_NAME}
    SYNC_INCLUDE_OPTIONS EXCLUDE "singletone/.*"
)