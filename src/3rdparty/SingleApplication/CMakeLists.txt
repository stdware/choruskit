cmake_minimum_required(VERSION 3.12.0)

project(SingleApplication LANGUAGES CXX DESCRIPTION "Replacement for QtSingleApplication")

set(CMAKE_AUTOMOC ON)

add_library(${PROJECT_NAME} OBJECT
    singleapplication.h
    singleapplication.cpp
    singleapplication_p.h
    singleapplication_p.cpp
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Find dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)

set(QT_COMPONENTS Core Network)
set(QT_LIBRARIES Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

if(QAPPLICATION_CLASS STREQUAL QApplication)
    list(APPEND QT_COMPONENTS Widgets)
    list(APPEND QT_LIBRARIES Qt${QT_VERSION_MAJOR}::Widgets)
elseif(QAPPLICATION_CLASS STREQUAL QGuiApplication)
    list(APPEND QT_COMPONENTS Gui)
    list(APPEND QT_LIBRARIES Qt${QT_VERSION_MAJOR}::Gui)
else()
    set(QAPPLICATION_CLASS QCoreApplication)
endif()

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_COMPONENTS} REQUIRED)

option(SINGLEAPPLICATION_DOCUMENTATION "Generate Doxygen documentation" OFF)

if(SINGLEAPPLICATION_DOCUMENTATION)
    find_package(Doxygen)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBRARIES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE advapi32)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC QAPPLICATION_CLASS=${QAPPLICATION_CLASS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_NO_CAST_TO_ASCII
    QT_NO_CAST_FROM_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_USE_QSTRINGBUILDER
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    QT_NO_KEYWORDS
    QT_NO_FOREACH
)

if(DOXYGEN_FOUND)
    # Doxygen theme
    include(FetchContent)
    FetchContent_Declare(DoxygenAwesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG 4cd62308d825fe0396d2f66ffbab45d0e247724c # 2.0.3
    )
    FetchContent_MakeAvailable(DoxygenAwesome)
    FetchContent_GetProperties(DoxygenAwesome SOURCE_DIR DoxygenAwesome_SOURCE_DIR)

    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_HTML_HEADER ${DoxygenAwesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${DoxygenAwesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${DoxygenAwesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${DoxygenAwesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${DoxygenAwesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )

    doxygen_add_docs(${PROJECT_NAME}Documentation
        singleapplication.h
        CHANGELOG.md
        Windows.md
        README.md
    )
endif()
