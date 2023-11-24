project(CkSVSBasic
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} SHARED AUTOGEN
    SOURCES ${_src}
    QT_LINKS Core
    QT_INCLUDE_PRIVATE Core
    INCLUDE_PRIVATE core
    SYNC_INCLUDE_PREFIX SVSBasic
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)