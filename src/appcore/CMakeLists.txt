project(CkAppCore
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

find_package(ExtensionSystem REQUIRED)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} SHARED AUTOGEN
    SOURCES ${_src}
    LINKS
        qtmediate::Widgets
        ExtensionSystem::ExtensionSystem
    QT_LINKS
        Core Widgets Network
    QT_INCLUDE_PRIVATE
        Core Gui Widgets
    INCLUDE_PRIVATE
        *
    FEATURES
        cxx_std_17
    SYNC_INCLUDE_PREFIX
        CoreApi
    SYNC_INCLUDE_OPTIONS
        EXCLUDE "internal/.+"
)