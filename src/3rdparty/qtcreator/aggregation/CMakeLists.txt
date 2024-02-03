project(Aggregation VERSION ${QT_CREATOR_VERSION} LANGUAGES CXX)

file(GLOB _src *.h *.cpp)

configure_qtc_library(${PROJECT_NAME}
    SOURCES ${_src}
    QT_LINKS Core
)