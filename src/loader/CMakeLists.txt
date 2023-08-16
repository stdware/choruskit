project(CkAppLoad
    VERSION 0.0.0.1
    LANGUAGES CXX
)

return()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} STATIC ${_src})

find_package(QasTool REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC
    QMWidgets ExtensionSystem qBreakpad qastool::core
    Qt${QT_DEFAULT_MAJOR_VERSION}::Core
    Qt${QT_DEFAULT_MAJOR_VERSION}::Widgets
    Qt${QT_DEFAULT_MAJOR_VERSION}::Network
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SingleApplication
)

if(WIN32)
    ck_attach_win_rc_file(${PROJECT_NAME}
        VENDOR "Sine Striker"
    )
endif()

include(config/qasc.cmake)