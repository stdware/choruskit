project(CkAppCore
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

ck_add_qt_module(_qt_libs Core Widgets Network)
ck_add_qt_private_inc(_qt_includes Core Gui Widgets)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} SHARED ${_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE CKAPPCORE_LIBRARY)

target_link_libraries(${PROJECT_NAME} PUBLIC qtmediate::WidgetTools ExtensionSystem ${_qt_libs})
target_link_libraries(${PROJECT_NAME} PRIVATE SingleApplication)

target_include_directories(${PROJECT_NAME} PRIVATE ${_qt_includes} include/CoreApi src)

choruskit_include_directories(${PROJECT_NAME} include)

if(WIN32)
    ck_attach_win_rc_file(${PROJECT_NAME}
        VENDOR "Sine Striker"
        DESCRIPTION "ChorusKit Framework Module"
    )
endif()

choruskit_install_headers(include)