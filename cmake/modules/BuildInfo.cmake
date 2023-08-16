include_guard(DIRECTORY)

include(${CMAKE_CURRENT_LIST_DIR}/Basic.cmake)

# Required Variables: CK_REPO_ROOT_DIR

#[[
Generate a configuration header.

    ck_generate_config_header(<def_list> <file>)
    
    def_list: A list of compile definitions, use "=" if there's a value

]] #
function(ck_generate_config_header _def_list _file)
    set(_options)
    set(_values)

    foreach(_item ${_def_list})
        string(REGEX MATCH "(.+)=(.+)" _ ${_item})

        if(${CMAKE_MATCH_COUNT} EQUAL 2)
            set(_key ${CMAKE_MATCH_1})
            set(_val ${CMAKE_MATCH_2})
            _ck_append_define_line(${_key} ${_val} _values)
            continue()
        endif()

        list(APPEND _options "#define ${_item}")
    endforeach()

    string(JOIN "\n" _content "#pragma once" "" ${_options} "" ${_values} "")
    file(GENERATE OUTPUT ${_file} CONTENT ${_content})
endfunction()

#[[
Generate a build information header.

    ck_generate_build_info_header(<file>)
    
    def_list: A list of compile definitions, use "=" if there's a value

]] #
function(ck_generate_build_info_header _file)
    set(_git_branch "unknown")
    set(_git_hash "unknown")

    find_package(Git QUIET)

    if(Git_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%H
            OUTPUT_VARIABLE _git_hash
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY ${CK_REPO_ROOT_DIR}
            COMMAND_ERROR_IS_FATAL ANY
        )

        execute_process(
            COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
            OUTPUT_VARIABLE _git_branch
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY ${CK_REPO_ROOT_DIR}
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()

    set(_compiler_name unknown)

    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set(_compiler_name "Clang")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(_compiler_name "GCC")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(_compiler_name "MSVC")
    elseif(CMAKE_CXX_COMPILER_ID)
        set(_compiler_name ${CMAKE_CXX_COMPILER_ID})
    endif()

    set(_compiler_arch ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID})

    if(NOT _compiler_arch)
        string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} _compiler_arch)
    endif()

    set(_compiler_version ${CMAKE_CXX_COMPILER_VERSION})

    if(NOT _compiler_version)
        set(_compiler_version 0)
    endif()

    string(TIMESTAMP _build_time "%Y/%m/%d %H:%M:%S")
    string(TIMESTAMP _build_year "%Y")

    set(_values)
    _ck_append_define_line(CHORUSKIT_BUILD_COMPILER_ID ${_compiler_name} _values)
    _ck_append_define_line(CHORUSKIT_BUILD_COMPILER_VERSION ${_compiler_version} _values)
    _ck_append_define_line(CHORUSKIT_BUILD_COMPILER_ARCH ${_compiler_arch} _values)
    _ck_append_define_line(CHORUSKIT_BUILD_DATE_TIME ${_build_time} _values)
    _ck_append_define_line(CHORUSKIT_BUILD_YEAR ${_build_year} _values)
    _ck_append_define_line(CHORUSKIT_GIT_COMMIT_HASH ${_git_hash} _values)
    _ck_append_define_line(CHORUSKIT_GIT_BRANCH ${_git_branch} _values)

    string(JOIN "\n" _content "#pragma once" "" ${_values} "")
    file(GENERATE OUTPUT ${_file} CONTENT ${_content})
endfunction()

# ----------------------------------
# Private functions
# ----------------------------------
function(_ck_append_define_line _key _val _out)
    string(LENGTH ${_key} _len)
    math(EXPR _cnt "40-${_len}")
    string(REPEAT " " ${_cnt} _spaces)
    list(APPEND ${_out} "#define ${_key}${_spaces}\"${_val}\"")
    set(${_out} ${${_out}} PARENT_SCOPE)
endfunction()