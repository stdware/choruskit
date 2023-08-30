project(CkLoader
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

ck_add_qt_module(_qt_libs Core Widgets)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} STATIC ${_src})

find_package(QasTool REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC
    qtmediate::Widgets ChorusKit::ExtensionSystem qastool::core
    ${_qt_libs}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ChorusKit::SingleApplication
)

choruskit_include_directories(${PROJECT_NAME} include)

choruskit_install_headers(include)

include(src/config/qasc.cmake)