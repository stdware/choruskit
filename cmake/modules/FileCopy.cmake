include_guard(DIRECTORY)

# Required Variables: CK_CMAKE_MODULES_DIR, CK_BUILD_MAIN_DIR, CK_BUILD_SHARE_DIR

#[[
Add a resources copying command after building the target.

    ck_add_attached_files(<target>
        [SKIP_BUILD] [SKIP_INSTALL]
        SRC <files1...> DEST <dir1>
        SRC <files2...> DEST <dir2> ...
    )
    
    SRC: source files or directories, use "*" to collect all items in directory
    DEST: destination directory, can be a generator expression
]] #
function(ck_add_attached_files _target)
    set(options SKIP_BUILD SKIP_INSTALL)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_error)
    _ck_resolve_src_dest("${FUNC_UNPARSED_ARGUMENTS}" _result _error)

    if(_error)
        message(FATAL_ERROR "ck_add_attached_files: ${_error}")
    endif()

    set(_options)

    if(FUNC_SKIP_BUILD)
        list(APPEND _options SKIP_BUILD)
    elseif(FUNC_SKIP_INSTALL)
        list(APPEND _options SKIP_INSTALL)
    endif()

    foreach(_item ${_result})
        list(POP_BACK _item _dest)
        _ck_add_copy_command(${_target} "$<TARGET_FILE_DIR:${_target}>" "${_item}" ${_dest} ${_options})
    endforeach()
endfunction()

#[[
Add a resources copying command for whole project.

    ck_add_shared_files(
        [SKIP_BUILD] [SKIP_INSTALL]
        SRC <files1...> DEST <dir1>
        SRC <files2...> DEST <dir2> ...
    )

    SRC: source files or directories, use "*" to collect all items in directory
    DEST: destination directory, can be a generator expression
    
    Related Targets:
        ChorusKit_CopySharedFiles
]] #
function(ck_add_shared_files)
    set(options SKIP_BUILD SKIP_INSTALL)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_error)
    _ck_resolve_src_dest("${FUNC_UNPARSED_ARGUMENTS}" _result _error)

    if(_error)
        message(FATAL_ERROR "ck_add_shared_files: ${_error}")
    endif()

    set(_options)

    if(FUNC_SKIP_BUILD)
        list(APPEND _options SKIP_BUILD)
    elseif(FUNC_SKIP_INSTALL)
        list(APPEND _options SKIP_INSTALL)
    endif()

    foreach(_item ${_result})
        list(POP_BACK _item _dest)

        string(RANDOM LENGTH 8 _rand)
        set(_attach_target shared_copy_command_${_rand})
        add_custom_target(${_attach_target})

        if(TARGET ChorusKit_CopySharedFiles)
            add_dependencies(ChorusKit_CopySharedFiles ${_attach_target})
        endif()

        _ck_add_copy_command(${_attach_target} ${CK_BUILD_SHARE_DIR} "${_item}" ${_dest} ${_options})
    endforeach()
endfunction()

# ----------------------------------
# Private functions
# ----------------------------------
function(_ck_resolve_src_dest _args _result _error)
    # State Machine
    set(_src)
    set(_dest)
    set(_status NONE) # NONE, SRC, DEST
    set(_count 0)

    set(_list)

    foreach(_item ${_args})
        if(${_item} STREQUAL SRC)
            if(${_status} STREQUAL NONE)
                set(_src)
                set(_status SRC)
            elseif(${_status} STREQUAL DEST)
                set(${_error} "missing directory name after DEST!" PARENT_SCOPE)
                return()
            else()
                set(${_error} "missing source files after SRC!" PARENT_SCOPE)
                return()
            endif()
        elseif(${_item} STREQUAL DEST)
            if(${_status} STREQUAL SRC)
                set(_status DEST)
            elseif(${_status} STREQUAL DEST)
                set(${_error} "missing directory name after DEST!" PARENT_SCOPE)
                return()
            else()
                set(${_error} "no source files specified for DEST!" PARENT_SCOPE)
                return()
            endif()
        else()
            if(${_status} STREQUAL NONE)
                set(${_error} "missing SRC or DEST token!" PARENT_SCOPE)
                return()
            elseif(${_status} STREQUAL DEST)
                if(NOT _src)
                    set(${_error} "no source files specified for DEST!" PARENT_SCOPE)
                    return()
                endif()

                set(_status NONE)
                math(EXPR _count "${_count} + 1")

                string(JOIN "\\;" _src_str ${_src})
                list(APPEND _list "${_src_str}\\;${_item}")
            else()
                get_filename_component(_path ${_item} ABSOLUTE)
                list(APPEND _src ${_path})
            endif()
        endif()
    endforeach()

    if(${_status} STREQUAL SRC)
        set(${_error} "missing DEST after source files!" PARENT_SCOPE)
        return()
    elseif(${_status} STREQUAL DEST)
        set(${_error} "missing directory name after DEST!" PARENT_SCOPE)
        return()
    elseif(${_count} STREQUAL 0)
        set(${_error} "no files specified!" PARENT_SCOPE)
        return()
    endif()

    set(${_result} "${_list}" PARENT_SCOPE)
endfunction()

function(_ck_add_copy_command _target _base_dir _src _dest)
    set(options SKIP_BUILD SKIT_INSTALL)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    ck_has_genex(${_dest} _has_genex)

    if(_has_genex)
        set(_path ${_dest})
    else()
        if(IS_ABSOLUTE ${_dest})
            set(_path ${_dest})
        else()
            set(_path ${_base_dir}/${_dest})
        endif()
    endif()

    foreach(_src_item ${_src})
        get_filename_component(_full_path ${_src_item} ABSOLUTE)

        # Add a post target to handle unexpected delete
        if(NOT FUNC_SKIP_BUILD)
            add_custom_command(TARGET ${_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                -D "src=${_full_path}"
                -D "dest=${_path}"
                -P "${CK_CMAKE_MODULES_DIR}/commands/CopyIfDifferent.cmake"
            )
        endif()

        if(NOT FUNC_SKIP_INSTALL)
            string(REPLACE "\\" "/" _install_prefix ${CMAKE_INSTALL_PREFIX})
            install(CODE "
                file(RELATIVE_PATH _rel_path \"${CK_BUILD_MAIN_DIR}\" \"${_path}\")
                execute_process(
                    COMMAND \"${CMAKE_COMMAND}\"
                    -D \"src=${_full_path}\"
                    -D \"dest=${_install_prefix}/\${_rel_path}\"
                    -P \"${CK_CMAKE_MODULES_DIR}/commands/CopyIfDifferent.cmake\"
                    COMMAND_ERROR_IS_FATAL ANY
                )
            ")
        endif()
    endforeach()
endfunction()