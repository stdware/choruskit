project(CkSVSBasic
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

ck_add_qt_module(_qt_libs Core)
ck_add_qt_private_inc(_qt_includes Core)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

choruskit_add_library(${PROJECT_NAME} SHARED ${_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE CKSVSBASIC_LIBRARY)

target_link_libraries(${PROJECT_NAME} PUBLIC ${_qt_libs})

target_include_directories(${PROJECT_NAME} PRIVATE ${_qt_includes} include/SVSBasic src)

choruskit_include_directories(${PROJECT_NAME} include)

if(WIN32)
    string(TIMESTAMP _year "%Y")
    qtmediate_add_win_rc(${PROJECT_NAME}
        COPYRIGHT "Copyright ${CK_DEV_START_YEAR}-${_year} OpenVPI"
        DESCRIPTION "ChorusKit Library"
    )
endif()

choruskit_install_headers(include)