set(QT_QMAKE_EXECUTABLE)

function(_find_qmake _out)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS ${_module} QUIET)

    if(NOT QT_FOUND)
        return()
    endif()

    set(_dir ${Qt${QT_VERSION_MAJOR}_DIR})

    while(TRUE)
        if(${_dir} MATCHES "(\\\\|\\/)$")
            set(_bin_path ${_dir}bin)
        else()
            set(_bin_path ${_dir}/bin)
        endif()

        if(IS_DIRECTORY ${_bin_path})
            set(_paths ${_bin_path}/qmake)

            if(WIN32)
                set(_qmake_path ${_bin_path}/qmake.exe)
            else()
                set(_qmake_path ${_bin_path}/qmake)
            endif()

            if(EXISTS ${_qmake_path})
                set(${_out} ${_qmake_path} PARENT_SCOPE)
                break()
            endif()
        endif()

        if("${_dir}" MATCHES "^[A-Z]:(\\\\|\\/)$")
            break()
        endif()

        get_filename_component(_dir ${_dir} DIRECTORY)
    endwhile()
endfunction()

_find_qmake(QT_QMAKE_EXECUTABLE)