project(CkAppCore
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

find_package(stdcorelib CONFIG REQUIRED)

choruskit_add_library(${PROJECT_NAME} SHARED AUTOGEN
    SOURCES ${_src}
    QT_LINKS
        Core Widgets Network Qml
    QT_INCLUDE_PRIVATE
        Core Gui Widgets Qml
    LINKS_PRIVATE
        stdcorelib::stdcorelib
    INCLUDE_PRIVATE
        *
    FEATURES
        cxx_std_17
    SYNC_INCLUDE_PREFIX
        CoreApi
)