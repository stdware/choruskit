project(CkSVSWidgets
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} SHARED AUTOGEN
    SOURCES ${_src}
    LINKS qtmediate::Widgets
    QT_LINKS Core Gui Widgets
    QT_INCLUDE_PRIVATE Core Gui Widgets
    INCLUDE_PRIVATE itemmodel widgets
    SYNC_INCLUDE_PREFIX SVSWidgets
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)