project(qBreakpad VERSION 0.2.1.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(unofficial-breakpad CONFIG REQUIRED)

include(../config.cmake)

file(GLOB_RECURSE _src *.h *.cpp)

# This library is released under LGPL 3.0 license,
# We static link it for convenience, so that ChorusKit is released under LGPL 3.0 too,
choruskit_add_library(${PROJECT_NAME} STATIC ${_src}
    LINKS_PRIVATE unofficial::breakpad::libbreakpad unofficial::breakpad::libbreakpad_client
    QT_LINKS Core Network
)