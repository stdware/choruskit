include_guard(DIRECTORY)

include(${CMAKE_CURRENT_LIST_DIR}/Basic.cmake)

# Required Variables: CK_APPLICATION_NAME, CK_CMAKE_SOURCE_DIR, CK_INSTALL_INCLUDE_DIR

#[[
Add files with specified patterns to list.

    ck_add_files(<list> PATTERNS <patterns...>
        [CURRENT] [SUBDIRS] [ALLDIRS] [DIRS dirs]
        [FILTER_PLATFORM]
    )

    Directory arguments or options:
        CURRENT: consider only current directory
        SUBDIRS: consider all subdirectories recursively
        ALLDIRS: consider both `CURRENT` and `SUBDIRS`
        DIRS: consider extra directories recursively
    
    Filter options:
        FILTER_PLATFORM: exclude files like `*_win.xxx` on Unix aand `*_unix.xxx` on Windows
]] #
function(ck_add_files _var)
    set(options CURRENT SUBDIRS ALLDIRS)
    set(oneValueArgs)
    set(multiValueArgs PATTERNS DIRS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT FUNC_PATTERNS)
        message(FATAL_ERROR "ck_add_files: PATTERNS not specified!")
    endif()

    set(_src)

    # Add current dir
    if(FUNC_CURRENT OR FUNC_ALLDIRS)
        set(_tmp_patterns)

        foreach(_pat ${FUNC_PATTERNS})
            list(APPEND _tmp_patterns ${CMAKE_CURRENT_SOURCE_DIR}/${_pat})
        endforeach()

        _ck_list_add_flatly(_src ${_tmp_patterns})
        unset(_tmp_patterns)
    endif()

    # Add sub dirs
    if(FUNC_SUBDIRS OR FUNC_ALLDIRS)
        ck_get_subdirs(_subdirs ABSOLUTE)
        set(_tmp_patterns)

        foreach(_subdir ${_subdirs})
            foreach(_pat ${FUNC_PATTERNS})
                list(APPEND _tmp_patterns ${_subdir}/${_pat})
            endforeach()
        endforeach()

        _ck_list_add_recursively(_src ${_tmp_patterns})
        unset(_tmp_patterns)
    endif()

    # Add other dirs recursively
    foreach(_dir ${FUNC_DIRS})
        set(_tmp_patterns)

        foreach(_pat ${FUNC_PATTERNS})
            list(APPEND _tmp_patterns ${_dir}/${_pat})
        endforeach()

        _ck_list_add_recursively(_src ${_tmp_patterns})
        unset(_tmp_patterns)
    endforeach()

    if(FUNC_FILTER_PLATFORM)
        if(WIN32)
            list(FILTER _src EXCLUDE REGEX ".*_(Unix|unix|Mac|mac|Linux|linux)\\.+")
        elseif(APPLE)
            list(FILTER _src EXCLUDE REGEX ".*_(Win|win|Windows|windows|Linux|linux)\\.+")
        else()
            list(FILTER _src EXCLUDE REGEX ".*_(Win|win|Windows|windows|Mac|mac)\\.+")
        endif()
    endif()

    set(${_var} ${_src} PARENT_SCOPE)
endfunction()

#[[
CMake target commands wrapper to add sources, links, includes.

    ck_target_components(<target>
        [SOURCES files...]

        [LINKS            libs...]
        [LINKS_PRIVATE    libs...]
        
        [DEFINES          defs...]
        [DEFINES_PRIVATE  defs...]

        [CCFLAGS          flags...]
        [CCFLAGS_PRIVATE  flags...]

        [QT_LINKS             modules...]
        [QT_LINKS_PRIVATE     modules...]
        [QT_INCLUDES_PRIVATE  modules...]

        [AUTO_INCLUDE_CURRENT]
        [AUTO_INCLUDE_SUBDIRS]
        [AUTO_INCLUDE_DIRS dirs...]

        [INCLUDE_CURRENT_PRIVATE]
        [INCLUDE_SUBDIRS_PRIVATE]
        [INCLUDE_PRIVATE dirs...]
    )
]] #
function(ck_target_components _target)
    set(options AUTO_INCLUDE_CURRENT INCLUDE_CURRENT_PRIVATE AUTO_INCLUDE_SUBDIRS INCLUDE_SUBDIRS_PRIVATE)
    set(oneValueArgs)
    set(multiValueArgs SOURCES
        LINKS LINKS_PRIVATE
        DEFINES DEFINES_PRIVATE
        CCFLAGS CCFLAGS_PUBLIC
        QT_LINKS QT_LINKS_PRIVATE QT_INCLUDES_PRIVATE
        AUTO_INCLUDE_DIRS INCLUDE_PRIVATE
    )
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ----------------- Template Begin -----------------
    target_sources(${_target} PRIVATE ${FUNC_SOURCES})
    target_include_directories(${_target} PRIVATE ${FUNC_INCLUDE_PRIVATE})

    target_link_libraries(${_target} PUBLIC ${FUNC_LINKS})
    target_link_libraries(${_target} PRIVATE ${FUNC_LINKS_PRIVATE})

    target_compile_definitions(${_target} PUBLIC ${FUNC_DEFINES})
    target_compile_definitions(${_target} PRIVATE ${FUNC_DEFINES_PRIVATE})

    target_compile_options(${_target} PUBLIC ${FUNC_CCFLAGS_PUBLIC})
    target_compile_options(${_target} PRIVATE ${FUNC_CCFLAGS})

    set(_qt_libs)
    ck_add_qt_module(_qt_libs ${FUNC_QT_LINKS})
    target_link_libraries(${_target} PUBLIC ${_qt_libs})

    set(_qt_libs_p)
    ck_add_qt_module(_qt_libs_p ${FUNC_QT_LINKS})
    target_link_libraries(${_target} PRIVATE ${_qt_libs_p})

    set(_qt_incs)
    ck_add_qt_private_inc(_qt_incs ${FUNC_QT_INCLUDES_PRIVATE})
    target_include_directories(${_target} PRIVATE ${_qt_incs})

    if(FUNC_INCLUDE_CURRENT_PRIVATE)
        target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    elseif(FUNC_AUTO_INCLUDE_CURRENT)
        file(RELATIVE_PATH _rel_path ${CK_CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(${_target}
            PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
            "$<INSTALL_INTERFACE:${CK_INSTALL_INCLUDE_DIR}/${_rel_path}>"
            "$<INSTALL_INTERFACE:${CK_INSTALL_INCLUDE_DIR}/${_rel_path}/..>"
        )
    endif()

    if(FUNC_AUTO_INCLUDE_SUBDIRS OR FUNC_INCLUDE_SUBDIRS_PRIVATE)
        ck_get_subdirs(_subdirs ABSOLUTE)

        if(FUNC_INCLUDE_SUBDIRS_PRIVATE)
            target_include_directories(${_target} PRIVATE ${_subdirs})
        else()
            foreach(_abs_dir ${_subdirs})
                file(RELATIVE_PATH _rel_path ${CK_CMAKE_SOURCE_DIR} ${_abs_dir})
                target_include_directories(${_target}
                    PUBLIC
                    "$<BUILD_INTERFACE:${_abs_dir}>"
                    "$<INSTALL_INTERFACE:${CHORUSKIT_RELATIVE_INCLUDE_DIR}/${_rel_path}>"
                )
            endforeach()
        endif()
    endif()

    if(FUNC_AUTO_INCLUDE_DIRS)
        foreach(_item ${FUNC_AUTO_INCLUDE_DIRS})
            get_filename_component(_abs_dir ${_item} ABSOLUTE)
            file(RELATIVE_PATH _rel_path ${CK_CMAKE_SOURCE_DIR} ${_abs_dir})
            target_include_directories(${_target}
                PUBLIC
                "$<BUILD_INTERFACE:${_abs_dir}>"
                "$<INSTALL_INTERFACE:${CHORUSKIT_RELATIVE_INCLUDE_DIR}/${_rel_path}>"
            )
        endforeach()
    endif()

    if(TARGET ChorusKit_Metadata)
        foreach(_item ${FUNC_LINKS} ${FUNC_LINKS_PRIVATE})
            if(NOT TARGET ${_item})
                message(WARNING "ck_target_components: target \"${_item}\" linked by \"${_target}\" not found.")
                continue()
            endif()

            ck_add_library_searching_paths(${_item})
        endforeach()
    endif()

    # ----------------- Template End -----------------
endfunction()

#[[
Collect targets of given types recursively in a directory.

    ck_collect_targets(<list> [DIR directory]
                    [EXECUTABLE] [SHARED] [STATIC] [UTILITY])

    If one or more types are specified, return targets matching the types.
    If no type is specified, return all targets.
]] #
function(ck_collect_targets _var)
    set(options EXECUTABLE SHARED STATIC UTILITY)
    set(oneValueArgs DIR)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_DIR)
        set(_dir ${FUNC_DIR})
    else()
        set(_dir ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    set(_tmp_targets)
    set(_targets)

    # Get targets
    ck_get_targets_recursive(_tmp_targets ${_dir})

    if(NOT FUNC_EXECUTABLE AND NOT FUNC_SHARED AND NOT FUNC_STATIC AND NOT FUNC_UTILITY)
        set(_targets ${_tmp_targets})
    else()
        # Filter targets
        foreach(_item ${_tmp_targets})
            get_target_property(_type ${_item} TYPE)

            if(${_type} STREQUAL "EXECUTABLE")
                if(FUNC_EXECUTABLE)
                    list(APPEND _targets ${_item})
                endif()
            elseif(${_type} STREQUAL "SHARED_LIBRARY")
                if(FUNC_SHARED)
                    list(APPEND _targets ${_item})
                endif()
            elseif(${_type} STREQUAL "STATIC_LIBRARY")
                if(FUNC_STATIC)
                    list(APPEND _targets ${_item})
                endif()
            elseif(${_type} STREQUAL "UTILITY")
                if(FUNC_UTILITY)
                    list(APPEND _targets ${_item})
                endif()
            endif()
        endforeach()
    endif()

    set(${_var} ${_targets} PARENT_SCOPE)
endfunction()

#[[
Set SKIP_AUTOMOC for all source files in specified directory.

    ck_dir_skip_automoc(<dir...>)
]] #
function(ck_dir_skip_automoc)
    foreach(_item ${ARGN})
        file(GLOB _src ${_item}/*.h ${_item}/*.cpp ${_item}/*.cc)
        set_source_files_properties(
            ${_src} PROPERTIES SKIP_AUTOMOC ON
        )
    endforeach()
endfunction()

#[[
Get a target's shared dependency locations.

    ck_get_target_dependencies(<list> <targets ...>)
]] #
function(ck_get_target_dependencies _list)
    set(_result)

    if(CMAKE_BUILD_TYPE)
        string(TOUPPER ${CMAKE_BUILD_TYPE} _config_upper)
    else()
        set(_config_upper DEBUG)
    endif()

    foreach(_item ${ARGN})
        get_target_property(_linked_libs ${_item} LINK_LIBRARIES)

        foreach(_item ${_linked_libs})
            get_target_property(_imported ${_item} IMPORTED)

            if(NOT _imported)
                continue()
            endif()

            message("${_item}")
            get_target_property(_path ${_item} LOCATION_${config_upper})

            if(NOT _path OR ${_path} IN_LIST _result)
                continue()
            endif()

            list(APPEND _result ${_path})
        endforeach()
    endforeach()

    set(${_list} ${_result} PARENT_SCOPE)
endfunction()

# ----------------------------------
# Private functions
# ----------------------------------
macro(_ck_list_prepend_prefix _list _prefix)
    foreach(_item ${ARGN})
        list(APPEND ${_list} ${_prefix}${_item})
    endforeach()
endmacro()

macro(_ck_list_remove_all _list1 _list2)
    foreach(_item ${${_list2}})
        list(REMOVE_ITEM ${_list1} ${_item})
    endforeach()
endmacro()

macro(_ck_list_add_flatly _list)
    set(_temp_list)
    file(GLOB _temp_list ${ARGN})
    list(APPEND ${_list} ${_temp_list})
    unset(_temp_list)
endmacro()

macro(_ck_list_add_recursively _list)
    set(_temp_list)
    file(GLOB_RECURSE _temp_list ${ARGN})
    list(APPEND ${_list} ${_temp_list})
    unset(_temp_list)
endmacro()