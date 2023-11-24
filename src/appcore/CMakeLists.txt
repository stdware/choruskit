project(CkAppCore
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} SHARED AUTOGEN
    SOURCES ${_src}
    LINKS qtmediate::WidgetTools ChorusKit::ExtensionSystem
    LINKS_PRIVATE SingleApplication
    QT_LINKS Core Widgets Network
    QT_INCLUDE_PRIVATE Core Gui Widgets
    INCLUDE_PRIVATE core editor managers window
    SYNC_INCLUDE_PREFIX CoreApi
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)