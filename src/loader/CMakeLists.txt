project(CkLoader
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

find_package(ExtensionSystem CONFIG REQUIRED)

choruskit_add_library(${PROJECT_NAME} STATIC NO_SYNC_INCLUDE AUTOGEN
    SOURCES ${_src}
    LINKS ExtensionSystem::ExtensionSystem
    LINKS_PRIVATE "$<BUILD_INTERFACE:SingleApplication::SingleApplication>" CkAppCore
    INCLUDE_PRIVATE include/CkLoader src
    QT_INCLUDE_PRIVATE Core Gui Widgets
    FEATURES cxx_std_17
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

if(CHORUSKIT_INSTALL)
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}
    )
endif()