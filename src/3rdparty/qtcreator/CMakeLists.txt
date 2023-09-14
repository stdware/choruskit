set(QT_CREATOR_VERSION 0.3.6.1)

set(QT_CREATOR_ENABLE_QML off)
set(QT_CREATOR_NO_FILENAME_LIST on)

function(configure_qtc_library _name)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    choruskit_add_library(${_name} SHARED)

    if(WIN32)
        qtmediate_add_win_rc(${_name}
            NAME ${_name}
            VERSION ${PROJECT_VERSION}
            DESCRIPTION "Qt Creator 3.6"
            COPYRIGHT "Copyright The Qt Company Ltd."
        )
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${_name}36
    )

    target_compile_definitions(${_name} PRIVATE _UNICODE UNICODE)

    choruskit_include_directories(${_name} . ..)
endfunction()

add_subdirectory(aggregation)
add_subdirectory(utils)
add_subdirectory(extensionsystem)