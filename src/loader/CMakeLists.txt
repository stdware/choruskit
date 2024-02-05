project(CkLoader
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

find_package(QasTool REQUIRED)
find_package(ExtensionSystem REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} STATIC NO_SYNC_INCLUDE
    SOURCES ${_src}
    LINKS qtmediate::Widgets ExtensionSystem::ExtensionSystem
    LINKS_PRIVATE qastool::core "$<BUILD_INTERFACE:SingleApplication::SingleApplication>"
    INCLUDE_PRIVATE include/CkLoader src
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

if(CHORUSKIT_INSTALL)
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}
    )
endif()

include(src/config/qasc.cmake)