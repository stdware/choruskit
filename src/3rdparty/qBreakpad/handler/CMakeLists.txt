project(qBreakpad VERSION 0.2.1.1 LANGUAGES CXX)

find_package(unofficial-breakpad CONFIG REQUIRED)

include(../config.cmake)

file(GLOB_RECURSE _src *.h *.cpp)

# This library is released under LGPL 3.0 license,
# We static link it for convenience, so that ChorusKit is released under LGPL 3.0 too,
add_library(${PROJECT_NAME} STATIC ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE
    unofficial::breakpad::libbreakpad
    unofficial::breakpad::libbreakpad_client
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_DEFAULT_MAJOR_VERSION}::Core
    Qt${QT_DEFAULT_MAJOR_VERSION}::Network
)