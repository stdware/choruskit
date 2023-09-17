include_guard(DIRECTORY)

#[[
Check if a variable is defined, throw error if not.

    ck_check_defined(var)
]] #
function(ck_check_defined _var)
    if(NOT DEFINED ${_var})
        message(FATAL_ERROR "\"${_var}\" is not defined")
    endif()
endfunction()

#[[
The macro works same as "option".

    ck_option(<name> <values...>)
]] #
macro(ck_option _name)
    if(NOT DEFINED ${_name})
        set(${_name} ${ARGN})
    endif()
endmacro()

#[[
Tell if there are any generator expressions in the string.

    ck_has_genex(<string> <output>)
]] #
function(ck_has_genex _str _out)
    string(GENEX_STRIP "${_str}" _no_genex)

    if("${_str}" STREQUAL "${_no_genex}")
        set(_res off)
    else()
        set(_res on)
    endif()

    set(${_out} ${_res} PARENT_SCOPE)
endfunction()

#[[
Get subdirectories' names or paths.

    ck_get_subdirs(<list>  
        [DIRECTORY dir]
        [EXCLUDE names...]
        [REGEX_INCLUDE exps...]
        [REGEX_EXLCUDE exps...]
        [RELATIVE path]
        [ABSOLUTE]
    )

    If `DIRECTORY` is not specified, consider `CMAKE_CURRENT_SOURCE_DIR`.
    If `RELATIVE` is specified, return paths evaluated as a relative path to it.
    If `ABSOLUTE` is specified, return absolute paths.
    If neither of them is specified, return names.
]] #
function(ck_get_subdirs _var)
    set(options ABSOLUTE)
    set(oneValueArgs DIRECTORY RELATIVE)
    set(multiValueArgs EXCLUDE REGEX_EXLCUDE)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_DIRECTORY)
        get_filename_component(_dir ${FUNC_DIRECTORY} ABSOLUTE)
    else()
        set(_dir ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    file(GLOB _subdirs LIST_DIRECTORIES true RELATIVE ${_dir} "${_dir}/*")

    if(FUNC_EXCLUDE)
        foreach(_exclude_dir ${FUNC_EXCLUDE})
            list(REMOVE_ITEM _subdirs ${_exclude_dir})
        endforeach()
    endif()

    if(FUNC_REGEX_INCLUDE)
        foreach(_exp ${FUNC_REGEX_INCLUDE})
            list(FILTER _subdirs INCLUDE REGEX ${_exp})
        endforeach()
    endif()

    if(FUNC_REGEX_EXCLUDE)
        foreach(_exp ${FUNC_REGEX_EXCLUDE})
            list(FILTER _subdirs EXCLUDE REGEX ${_exp})
        endforeach()
    endif()

    set(_res)

    if(FUNC_RELATIVE)
        get_filename_component(_relative ${FUNC_RELATIVE} ABSOLUTE)
    else()
        set(_relative)
    endif()

    foreach(_sub ${_subdirs})
        if(IS_DIRECTORY ${_dir}/${_sub})
            if(FUNC_ABSOLUTE)
                list(APPEND _res ${_dir}/${_sub})
            elseif(_relative)
                file(RELATIVE_PATH _rel_path ${_relative} ${_dir}/${_sub})
                list(APPEND _res ${_rel_path})
            else()
                list(APPEND _res ${_sub})
            endif()
        endif()
    endforeach()

    set(${_var} ${_res} PARENT_SCOPE)
endfunction()

#[[
Add all or partial sub-directories to buildsystem

    ck_add_subdirectories([ALL] [<dirs...>])
]] #
function(ck_add_subdirectories)
    set(options ALL)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    ck_get_subdirs(_dirs)

    if(FUNC_ALL)
        foreach(_dir ${_dirs})
            add_subdirectory(${_dir})
        endforeach()
    else()
        set(_apps_lower)

        foreach(_item ${FUNC_UNPARSED_ARGUMENTS})
            string(TOLOWER ${_item} _lower)
            list(APPEND _apps_lower ${_lower})
        endforeach()

        foreach(_dir ${_dirs})
            string(TOLOWER ${_dir} _lower)
            list(FIND _apps_lower ${_lower} _out)

            if(_out GREATER_EQUAL 0)
                add_subdirectory(${_dir})
            endif()
        endforeach()
    endif()
endfunction()

#[[
Get all targets in a directory recursively.

    ck_get_targets_recursive(<VAR> <dir>)
]] #
macro(ck_get_targets_recursive _targets _dir)
    get_property(_subdirs DIRECTORY ${_dir} PROPERTY SUBDIRECTORIES)

    foreach(_subdir ${_subdirs})
        _ck_get_targets_recursive(${_targets} ${_subdir})
    endforeach()

    get_property(_current_targets DIRECTORY ${_dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${_targets} ${_current_targets})
endmacro()