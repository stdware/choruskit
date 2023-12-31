include_guard(DIRECTORY)

if(NOT DEFINED QTMEDIATE_CMAKE_MODULES_DIR)
    message(FATAL_ERROR "QTMEDIATE_CMAKE_MODULES_DIR not defined!")
endif()

include("${QTMEDIATE_CMAKE_MODULES_DIR}/QtMediateAPI.cmake")

if(NOT DEFINED CK_CMAKE_MODULES_DIR)
    set(CK_CMAKE_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()

if(NOT DEFINED CK_BUILD_MAIN_DIR)
    set(CK_BUILD_MAIN_DIR ${CMAKE_BINARY_DIR}/out-${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_BUILD_TYPE})
endif()

if(NOT DEFINED CK_RUN_SCRIPTS_VERBOSE)
    set(CK_RUN_SCRIPTS_VERBOSE off)
endif()

if(NOT DEFINED CK_ENABLE_DEVEL)
    set(CK_ENABLE_DEVEL off)
endif()

if(NOT DEFINED CK_ENABLE_CONSOLE)
    set(CK_ENABLE_CONSOLE on)
endif()

set(CK_APPLICATION_NAME ChorusKit)
set(CK_APPLICATION_VENDOR OpenVPI)
set(CK_DEV_START_YEAR 2019)
set(CK_INITIALIZED off)

#[[
Initialize ChorusKitApi global settings.

    ck_init_buildsystem(<name>
        [ROOT <dir>]
    )
]] #
macro(ck_init_build_system _app)
    set(options)
    set(oneValueArgs ROOT VENDOR)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Find QMake (Required)
    # We shouldn't call find_package in this environment, so we need to find program manually
    if(NOT DEFINED QT_QMAKE_EXECUTABLE)
        set(QT_QMAKE_EXECUTABLE)
        include(${CK_CMAKE_MODULES_DIR}/modules/FindQMake.cmake)
    endif()

    if(QT_QMAKE_EXECUTABLE)
        message(STATUS "Qmake found: ${QT_QMAKE_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Qmake not found")
    endif()

    if(CK_INITIALIZED)
        message(FATAL_ERROR "ck_init_build_system: build system has initialized")
    endif()

    set(CK_INITIALIZED on)

    # Meta
    set(CK_APPLICATION_NAME ${_app})
    set(CK_CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(CK_INSTALL_EXPORT ${_app}Targets)

    if(FUNC_ROOT)
        get_filename_component(CK_REPO_ROOT_DIR ${FUNC_ROOT} ABSOLUTE)
    else()
        set(CK_REPO_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    if(FUNC_VENDOR)
        set(CK_APPLICATION_VENDOR ${FUNC_VENDOR})
    endif()

    set(CK_ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/etc)

    set(CK_BUILD_INCLUDE_DIR ${CK_ARCHIVE_OUTPUT_PATH}/include)

    # Build
    if(APPLE)
        set(CK_BUILD_MAIN_DIR ${CK_BUILD_MAIN_DIR}/${_app}.app/Contents)

        set(_CK_BUILD_BASE_DIR ${CK_BUILD_MAIN_DIR})
        set(CK_BUILD_RUNTIME_DIR ${_CK_BUILD_BASE_DIR}/MacOS)
        set(CK_BUILD_LIBRARY_DIR ${_CK_BUILD_BASE_DIR}/Frameworks)
        set(CK_BUILD_PLUGINS_DIR ${_CK_BUILD_BASE_DIR}/Plugins)
        set(CK_BUILD_SHARE_DIR ${_CK_BUILD_BASE_DIR}/Resources)
        set(CK_BUILD_QT_CONF_DIR ${_CK_BUILD_BASE_DIR}/Resources)

        set(_CK_INSTALL_BASE_DIR ${_app}.app/Contents)
        set(CK_INSTALL_RUNTIME_DIR ${_CK_INSTALL_BASE_DIR}/MacOS)
        set(CK_INSTALL_LIBRARY_DIR ${_CK_INSTALL_BASE_DIR}/Frameworks)
        set(CK_INSTALL_PLUGINS_DIR ${_CK_INSTALL_BASE_DIR}/Plugins)
        set(CK_INSTALL_SHARE_DIR ${_CK_INSTALL_BASE_DIR}/Resources)
        set(CK_INSTALL_INCLUDE_DIR ${_CK_INSTALL_BASE_DIR}/Resources/include/${_app})
        set(CK_INSTALL_CMAKE_DIR ${_CK_INSTALL_BASE_DIR}/Resources/lib/cmake/${_app})

        set(CK_PLATFORM_NAME Macintosh)
        set(CK_PLATFORM_LOWER mac)
    else()
        set(CK_BUILD_RUNTIME_DIR ${CK_BUILD_MAIN_DIR}/bin)
        set(CK_BUILD_LIBRARY_DIR ${CK_BUILD_MAIN_DIR}/lib)
        set(CK_BUILD_PLUGINS_DIR ${CK_BUILD_MAIN_DIR}/lib/${_app}/plugins)
        set(CK_BUILD_SHARE_DIR ${CK_BUILD_MAIN_DIR}/share)
        set(CK_BUILD_QT_CONF_DIR ${CK_BUILD_MAIN_DIR}/bin)

        set(CK_INSTALL_RUNTIME_DIR bin)
        set(CK_INSTALL_LIBRARY_DIR lib)
        set(CK_INSTALL_PLUGINS_DIR ${CK_BUILD_MAIN_DIR}/Plugins)
        set(CK_INSTALL_SHARE_DIR share)
        set(CK_INSTALL_INCLUDE_DIR include/${_app})
        set(CK_INSTALL_CMAKE_DIR lib/cmake/${_app})

        if(WIN32)
            set(CK_PLATFORM_NAME Windows)
            set(CK_PLATFORM_LOWER win)
        elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
            set(LINUX true CACHE BOOL "Linux System" FORCE)
            set(CK_PLATFORM_NAME Linux)
            set(CK_PLATFORM_LOWER linux)
        else()
            message(FATAL_ERROR "Unsupported System !!!")
        endif()
    endif()

    # Store data during configuration
    add_custom_target(ChorusKit_Metadata)

    # Update all translations
    add_custom_target(ChorusKit_UpdateTranslations)

    # Release all translations
    add_custom_target(ChorusKit_ReleaseTranslations)

    # Copy global shared files
    add_custom_target(ChorusKit_CopySharedFiles)

    # Used ChorusKit Metadata Keys:
    # CONFIG_DEFINITIONS
    # LIBRARY_SEARCHING_PATHS
    # APPLICATION_PLUGINS
    # APPLICATION_LIBRARIES
endmacro()

#[[
Do final work.

    ck_finish_build_system()
]] #
function(ck_finish_build_system)
    if(NOT CK_INITIALIZED)
        message(FATAL_ERROR "ck_finish_build_system: build system not initialized")
    endif()

    include(${CK_CMAKE_MODULES_DIR}/modules/BuildInfo.cmake)

    # Generate config header
    set(_config_header "${CK_BUILD_INCLUDE_DIR}/choruskit_config.h")

    get_target_property(_def_list ChorusKit_Metadata CONFIG_DEFINITIONS)

    if(_def_list)
        ck_generate_config_header("${_def_list}" "${_config_header}")
    else()
        file(WRITE ${_config_header} "")
    endif()

    # Generate build info header
    set(_buildinfo_header "${CK_BUILD_INCLUDE_DIR}/choruskit_buildinfo.h")
    ck_generate_build_info_header("${_buildinfo_header}")

    _ck_post_deploy()
endfunction()

#[[
Add compile definitions to ChorusKit config header.

    ck_add_definition(<key> [value])
]] #
function(ck_add_definition)
    set(_def)

    set(_list ${ARGN})
    list(LENGTH _list _len)

    if(${_len} EQUAL 1)
        set(_def ${_list})
    elseif(${_len} EQUAL 2)
        # Get key
        list(POP_FRONT _list _key)
        list(POP_FRONT _list _val)

        # Boolean
        string(TOLOWER ${_val} _val_lower)

        if(${_val_lower} STREQUAL "off" OR ${_val_lower} STREQUAL "false")
            return()
        elseif(${_val_lower} STREQUAL "on" OR ${_val_lower} STREQUAL "true")
            set(_def ${_key})
        else()
            set(_def "${_key}=${_val}")
        endif()
    else()
        message(FATAL_ERROR "ck_add_definition: called with incorrect number of arguments")
    endif()

    set_property(TARGET ChorusKit_Metadata APPEND PROPERTY CONFIG_DEFINITIONS "${_def}")
endfunction()

#[[
Add library searching paths to build system, only Windows need this function.

    ck_add_library_searching_paths(<path or target ...>)
]] #
function(ck_add_library_searching_paths)
    if(NOT WIN32)
        return()
    endif()

    get_target_property(_paths ChorusKit_Metadata LIBRARY_SEARCHING_PATHS)

    if(NOT _paths)
        set(_paths)
    endif()

    if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL Debug)
        set(_config_upper DEBUG)
    else()
        string(TOUPPER ${CMAKE_BUILD_TYPE} _config_upper)
    endif()

    foreach(_item ${ARGN})
        if(TARGET ${_item})
            # Resolve location
            get_target_property(_imported ${_item} IMPORTED)

            if(NOT _imported)
                continue()
            endif()

            get_target_property(_path ${_item} LOCATION_${_config_upper})

            if(NOT _path OR ${_path} IN_LIST _result)
                continue()
            endif()

            get_filename_component(_path ${_path} DIRECTORY)
            set(_item ${_path})
        endif()

        if(${_item} IN_LIST _paths)
            continue()
        endif()

        list(APPEND _paths ${_item})
    endforeach()

    set_target_properties(ChorusKit_Metadata PROPERTIES LIBRARY_SEARCHING_PATHS "${_paths}")
endfunction()

#[[
Add application target.

    ck_configure_application(<ICO ico> <ICNS icns>
        [SKIP_EXPORT]
        [NAME           name] 
        [VERSION        version] 
        [DESCRIPTION    desc]
    )

    ICO: set Windows icon file
    ICNS: set Mac icon file
]] #
function(ck_configure_application)
    set(options)
    set(oneValueArgs ICO ICNS)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_target ${CK_APPLICATION_NAME})

    add_executable(${_target})

    # Make location dependent executable, otherwise GNOME cannot recognize
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        target_link_options(${_target} PRIVATE "-no-pie")
    endif()

    string(TIMESTAMP _year "%Y")
    set(_copyright "Copyright ${CK_DEV_START_YEAR}-${_year} ${CK_APPLICATION_VENDOR}")

    if(APPLE)
        if(FUNC_ICNS)
            set(_icns ICON ${FUNC_ICNS})
        else()
            set(_icns)
        endif()

        # Add mac bundle
        qtmediate_add_mac_bundle(${_target}
            COPYRIGHT "${_copyright}"
            ${_icns}
            ${FUNC_UNPARSED_ARGUMENTS}
        )
        set_target_properties(${_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_MAIN_DIR}
        )
    else()
        if(WIN32)
            # Set windows application type
            if(NOT CK_ENABLE_CONSOLE)
                set_target_properties(${_target} PROPERTIES
                    WIN32_EXECUTABLE TRUE
                )
            endif()

            if(FUNC_ICO)
                set(_ico ICON ${FUNC_ICO})
            else()
                set(_ico)
            endif()

            # Add windows rc file
            qtmediate_add_win_rc(${_target}
                COPYRIGHT "${_copyright}"
                ${_ico}
                ${FUNC_UNPARSED_ARGUMENTS}
            )

            # Add manifest
            qtmediate_add_win_manifest(${_target}
                COPYRIGHT "${_copyright}"
                ${_ico}
                ${FUNC_UNPARSED_ARGUMENTS}
            )

            # Add shortcut
            qtmediate_create_win_shortcut(${_target} ${CK_BUILD_MAIN_DIR}
                OUTPUT_NAME "${_target}"
            )
        endif()

        set_target_properties(${_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_RUNTIME_DIR}
        )
    endif()

    if(FUNC_SKIP_EXPORT OR NOT CK_ENABLE_DEVEL)
        set(_export)
    else()
        set(_export EXPORT ${CK_INSTALL_EXPORT})
    endif()

    if(APPLE)
        # Install to .
        install(TARGETS ${_target}
            ${_export}
            DESTINATION . OPTIONAL
        )
    else()
        # Install to bin
        install(TARGETS ${_target}
            ${_export}
            DESTINATION ${CK_INSTALL_RUNTIME_DIR} OPTIONAL
        )
    endif()

    # Add post build events to distribute shared files
    add_custom_command(TARGET ${_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ChorusKit_CopySharedFiles
    )
endfunction()

#[[
Add an application plugin.

    ck_add_application_plugin(<target>
        [SKIP_EXPORT]   [NO_PLUGIN_JSON]
        [CATEGORY       category]
        [PLUGIN_JSON    plugin.json.in]
        [COMPAT_VERSION compat_version]
        [NAME           name] 
        [VERSION        version] 
        [DESCRIPTION    desc]
        [VENDOR         vendor]
        [MACRO_PREFIX   prefix]
        [TYPE_MACRO     macro]
        [LIBRARY_MACRO  macro]
    )

    NO_PLUGIN_JSON: skip configuring the plugin.json.in
    CATEGORY: set the sub-directory name for plugin to output, which is same as `PROJECT_NAME` by default
    PLUGIN_JSON: set the custom plugin.json.in file to configure, otherwise configure the plugin.json.in in currect directory
]] #
function(ck_add_application_plugin _target)
    set(options SKIP_EXPORT)
    set(oneValueArgs VENDOR PLUGIN_JSON)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Add Qt Moc
    _ck_set_cmake_autoxxx(on)

    # Add library target and attach definitions
    _ck_add_library_internal(${_target} SHARED ${FUNC_UNPARSED_ARGUMENTS})

    if(CK_INITIALIZED)
        add_library(${CK_APPLICATION_NAME}::${_target} ALIAS ${_target})
    endif()

    # Add target level dependency
    add_dependencies(${CK_APPLICATION_NAME} ${_target})

    # Set parsed name as output name if not set
    _ck_try_set_output_name(${_target} ${_target})

    qtmediate_set_value(_vendor FUNC_VENDOR ${CK_APPLICATION_VENDOR})

    if(WIN32)
        string(TIMESTAMP _year "%Y")

        set(_copyright "Copyright ${CK_DEV_START_YEAR}-${_year} ${_vendor}")

        # Add windows rc file
        qtmediate_add_win_rc(${_target}
            COPYRIGHT "${_copyright}"
            ${FUNC_UNPARSED_ARGUMENTS}
        )
    endif()

    # Configure plugin json if specified
    if(FUNC_PLUGIN_JSON)
        ck_configure_plugin_metadata(${_target} ${FUNC_PLUGIN_JSON} ${FUNC_UNPARSED_ARGUMENTS} VENDOR ${_vendor})
    elseif(NOT FUNC_NO_PLUGIN_JSON AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json.in)
        ck_configure_plugin_metadata(${_target} plugin.json.in ${FUNC_UNPARSED_ARGUMENTS} VENDOR ${_vendor})
    endif()

    # Configure plugin desc file
    set(_tmp_desc_file ${CMAKE_CURRENT_BINARY_DIR}/${_target}Metadata/plugin.json)
    _ck_configure_plugin_desc(${_tmp_desc_file} ${FUNC_UNPARSED_ARGUMENTS})
    ck_add_attached_files(${_target}
        SRC ${_tmp_desc_file} DEST .
    )

    qtmediate_set_value(_category FUNC_CATEGORY ${_target})

    set(_build_output_dir ${CK_BUILD_PLUGINS_DIR}/${_category})
    set(_install_output_dir ${CK_INSTALL_PLUGINS_DIR}/${_category})

    # Set output directories
    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${_build_output_dir}
        LIBRARY_OUTPUT_DIRECTORY ${_build_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY ${_build_output_dir}
    )

    # Install target
    if(FUNC_SKIP_EXPORT)
        set(_export)
    else()
        set(_export EXPORT ${CK_INSTALL_EXPORT})
    endif()

    if(CK_ENABLE_DEVEL)
        install(TARGETS ${_target}
            ${_export}
            RUNTIME DESTINATION ${_install_output_dir}
            LIBRARY DESTINATION ${_install_output_dir}
            ARCHIVE DESTINATION ${_install_output_dir}
        )
    else()
        install(TARGETS ${_target}
            ${_export}
            RUNTIME DESTINATION ${_install_output_dir}
            LIBRARY DESTINATION ${_install_output_dir}
        )
    endif()

    set_property(TARGET ChorusKit_Metadata APPEND PROPERTY APPLICATION_PLUGINS ${_target})
endfunction()

#[[
Configure plugin metadata json.

    ck_configure_plugin_metadata(<target>
        [NAME               name            ] 
        [VERSION            version         ] 
        [COMPAT_VERSION     compat_version  ]
        [VENDOR             vendor          ]
    )

    NAME: to be removed
    COMPAT_VERSION: set the PLUGIN_METADATA_COMPAT_VERSION variable as compatible version
    VERSION: set the PLUGIN_METADATA_VERSION variable as version
    VENDOR: set the PLUGIN_METADATA_VENDOR variable as vendor
]] #
function(ck_configure_plugin_metadata _target _plugin_json)
    set(options)
    set(oneValueArgs NAME VERSION COMPAT_VERSION VENDOR)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Set plugin metadata
    qtmediate_set_value(_name FUNC_NAME ${PROJECT_NAME}) # to be removed
    qtmediate_set_value(_version FUNC_VERSION ${PROJECT_VERSION})
    qtmediate_set_value(_compat_version FUNC_COMPAT_VERSION "0.0.0.0")
    qtmediate_set_value(_vendor FUNC_VENDOR "${CK_APPLICATION_VENDOR}")

    # Fix version
    qtmediate_parse_version(_ver ${_version})
    set(PLUGIN_METADATA_VERSION ${_ver_1}.${_ver_2}.${_ver_3}_${_ver_4})

    qtmediate_parse_version(_compat ${_compat_version})
    set(PLUGIN_METADATA_COMPAT_VERSION ${_compat_1}.${_compat_2}.${_compat_3}_${_compat_4})
    set(PLUGIN_METADATA_VENDOR ${_vendor})

    # Get year
    string(TIMESTAMP _year "%Y")
    set(PLUGIN_METADATA_YEAR "${_year}")

    configure_file(
        ${_plugin_json}
        ${CMAKE_CURRENT_BINARY_DIR}/QtPluginMetadata/plugin.json
        @ONLY
    )
    target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/QtPluginMetadata)
endfunction()

#[[
Add a library, default to static library.

    ck_add_library(<target>
        [SHARED] [AUTOGEN] [SKIP_INSTALL] [SKIP_EXPORT]
        [NAME           name] 
        [VERSION        version] 
        [DESCRIPTION    desc]
        [COPYRIGHT copyright | VENDOR vendor]
        [MACRO_PREFIX   prefix]
        [TYPE_MACRO     macro]
        [LIBRARY_MACRO  macro]
    )

    SHARED: build shared library, otherwise build static library if not set
    AUTOGEN: set CMAKE_AUTOMOC, CMAKE_AUTOUIC, CMAKE_AUTORCC
    SKIP_INSTALL: skip install the test target
    SKIP_EXPORT: skip export the test target to installed cmake package
    NAME: set output file name and name property in Windows RC, which is same as target name by default
    VERSION: set version property in Windows RC, which is same as `PROJECT_VERSION` by default
    DESCRIPTION: set description property in Windows RC, which is same as target name by default
    VENDOR: set vendor with default copyright string in Windows RC, which would be `OpenVPI` by defult
    COPYRIGHT: set custom copyright string in Windows RC, and VENDOR will be ignored
    MACRO_PREFIX: set a prefered prefix to define library type macro and library internal macro,
                  otherwise the fallback prefix is same as NAME value if NAME is set,
                  otherwise the fallback prefix is same as target name
    TYPE_MACRO: set custom library type macro, otherwise the fallback is `prefix`_STATIC or `prefix`_SHARED
    LIBRARY_MACRO: set custom library internal macro, otherwise the fallback is `prefix`_LIBRARY
]] #
function(ck_add_library _target)
    set(options AUTOGEN SKIP_INSTALL SKIP_EXPORT)
    set(oneValueArgs COPYRIGHT VENDOR)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Add Qt Moc
    if(FUNC_AUTOGEN)
        _ck_set_cmake_autoxxx(on)
    endif()

    # Add library target and attach definitions
    _ck_add_library_internal(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Get target type
    _ck_check_shared_library(${_target} _shared)

    if(CK_INITIALIZED)
        add_library(${CK_APPLICATION_NAME}::${_target} ALIAS ${_target})
    endif()

    # Add windows rc file
    if(WIN32)
        string(TIMESTAMP _year "%Y")

        if(FUNC_COPYRIGHT)
            set(_copyright ${FUNC_COPYRIGHT})
        else()
            qtmediate_set_value(_vendor FUNC_VENDOR "${CK_APPLICATION_VENDOR}")
            set(_copyright "Copyright ${CK_DEV_START_YEAR}-${_year} ${_vendor}")
        endif()

        get_target_property(_type ${_target} TYPE)

        if(_shared)
            qtmediate_add_win_rc(${_target}
                COPYRIGHT "${_copyright}"
                ${FUNC_UNPARSED_ARGUMENTS}
            )
        endif()
    endif()

    set(_build_output_dir ${CK_BUILD_LIBRARY_DIR}/${CK_APPLICATION_NAME})
    set(_install_output_dir ${CK_INSTALL_LIBRARY_DIR}/${CK_APPLICATION_NAME})

    # Set output directories
    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_RUNTIME_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${_build_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY ${_build_output_dir}
    )

    # Install target
    if(FUNC_SKIP_EXPORT OR NOT CK_ENABLE_DEVEL)
        set(_export)
    else()
        set(_export EXPORT ${CK_INSTALL_EXPORT})
    endif()

    if(NOT FUNC_SKIP_INSTALL)
        if(CK_ENABLE_DEVEL)
            install(TARGETS ${_target}
                ${_export}
                RUNTIME DESTINATION ${CK_INSTALL_RUNTIME_DIR}
                LIBRARY DESTINATION ${_install_output_dir}
                ARCHIVE DESTINATION ${_install_output_dir}
            )
        elseif(_shared)
            install(TARGETS ${_target}
                ${_export}
                RUNTIME DESTINATION ${CK_INSTALL_RUNTIME_DIR}
                LIBRARY DESTINATION ${_install_output_dir}
            )
        endif()
    endif()

    set_property(TARGET ChorusKit_Metadata APPEND PROPERTY APPLICATION_LIBRARIES ${_target})
endfunction()

#[[
Add plain executable target, won't be installed.

    ck_add_executable(<target> [sources]
        [AUTOGEN] [CONSOLE] [WINDOWS]
    )

    AUTOGEN: set CMAKE_AUTOMOC, CMAKE_AUTOUIC, CMAKE_AUTORCC
    CONSOLE: build console application on Windows
    WINDOWS: build windows application on Windows
]] #
function(ck_add_executable _target)
    set(options AUTOGEN CONSOLE WINDOWS)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_AUTOGEN)
        _ck_set_cmake_autoxxx(on)
    endif()

    add_executable(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    if(WIN32 AND NOT FUNC_CONSOLE)
        if(FUNC_WINDOWS)
            set_target_properties(${_target} PROPERTIES WIN32_EXECUTABLE TRUE)
        else()
            # Set windows application type
            if(NOT CK_ENABLE_CONSOLE)
                set_target_properties(${_target} PROPERTIES
                    WIN32_EXECUTABLE TRUE
                )
            endif()
        endif()
    endif()

    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_RUNTIME_DIR}
    )

    set_property(TARGET ChorusKit_Metadata APPEND PROPERTY PLAIN_EXECUTABLES ${_target})
endfunction()

#[[
Get shorter version.

    ck_get_short_version(<VAR> <version> <count>)
]] #
function(ck_get_short_version _var _version _count)
    qtmediate_parse_version(FUNC ${_version})

    set(_list)

    foreach(_i RANGE 1 ${_count})
        list(APPEND _list ${FUNC_${_i}})
    endforeach()

    string(JOIN "." _short_version ${_list})
    set(${_var} ${_short_version} PARENT_SCOPE)
endfunction()

# ----------------------------------
# ChorusKit Private API
# ----------------------------------
macro(_ck_set_cmake_autoxxx _val)
    set(CMAKE_AUTOMOC ${_val})
    set(CMAKE_AUTOUIC ${_val})
    set(CMAKE_AUTORCC ${_val})
endmacro()

function(_ck_add_library_internal _target)
    set(options SHARED)
    set(oneValueArgs NAME MACRO_PREFIX LIBRARY_MACRO TYPE_MACRO)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_MACRO_PREFIX)
        set(_prefix ${FUNC_MACRO_PREFIX})
    elseif(FUNC_NAME)
        string(TOUPPER ${FUNC_NAME} _prefix)
    else()
        string(TOUPPER ${_target} _prefix)
    endif()

    if(FUNC_LIBRARY_MACRO)
        set(_library_macro ${FUNC_LIBRARY_MACRO})
    else()
        set(_library_macro ${_prefix}_LIBRARY)
    endif()

    if(FUNC_TYPE_MACRO)
        set(_type_macro ${FUNC_TYPE_MACRO})
    else()
        if(FUNC_SHARED)
            set(_type_macro ${_prefix}_SHARED)
        else()
            set(_type_macro ${_prefix}_STATIC)
        endif()
    endif()

    if(FUNC_SHARED)
        add_library(${_target} SHARED)
    else()
        add_library(${_target} STATIC)
    endif()

    if(FUNC_NAME)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${FUNC_NAME})
    endif()

    target_compile_definitions(${_target} PUBLIC ${_type_macro})
    target_compile_definitions(${_target} PRIVATE ${_library_macro})
endfunction()

function(_ck_try_set_output_name _target _name)
    get_target_property(_org ${_target} OUTPUT_NAME)

    if(NOT _org)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${_name})
    endif()
endfunction()

function(_ck_configure_plugin_desc _file)
    set(options ALL_FILES ALL_SUBDIRS)
    set(oneValueArgs NAME)
    set(multiValueArgs SUBDIRS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    qtmediate_set_value(_name FUNC_NAME ${PROJECT_NAME})

    set(_content "{\n    \"name\": \"${_name}\"")

    if(FUNC_ALL_FILES)
        set(_content "${_content},\n    \"allFiles\": true")
    endif()

    if(FUNC_ALL_SUBDIRS)
        set(_content "${_content},\n    \"allSubdirs\": true")
    endif()

    if(FUNC_SUBDIRS)
        string(JOIN "\",\n        \"" _json_arr_str ${FUNC_SUBDIRS})
        set(_content "${_content},\n    \"subdirs\": [\n        \"${_json_arr_str}\"\n    ]")
    endif()

    set(_content "${_content}\n}")

    file(GENERATE OUTPUT ${_file} CONTENT ${_content})
endfunction()

function(_ck_check_shared_library _target _out)
    set(_res off)
    get_target_property(_type ${_target} TYPE)

    if(${_type} STREQUAL SHARED_LIBRARY)
        set(_res on)
    endif()

    set(${_out} ${_res} PARENT_SCOPE)
endfunction()

function(_ck_try_set_output_name _target _name)
    get_target_property(_org ${_target} OUTPUT_NAME)

    if(NOT _org)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${_name})
    endif()
endfunction()

function(_ck_post_deploy)
    if(NOT Python_EXECUTABLE)
        # Python
        find_package(Python QUIET)

        if(Python_FOUND AND ${Python_VERSION} VERSION_GREATER_EQUAL 3.8)
            message(STATUS "Python found: ${Python_EXECUTABLE} (version ${Python_VERSION})")
        else()
            message(WARNING "Python not found, the installation maybe incomplete")
            return()
        endif()
    endif()

    add_custom_target(ChorusKit_AppLocalDeps DEPENDS ChorusKit_ReleaseTranslations)

    # Add application
    set(_binary_paths $<TARGET_FILE:${CK_APPLICATION_NAME}>)

    # Add plugins
    get_target_property(_plugin_list ChorusKit_Metadata APPLICATION_PLUGINS)

    if(_plugin_list)
        foreach(_item ${_plugin_list})
            list(APPEND _binary_paths $<TARGET_FILE:${_item}>)
        endforeach()
    endif()

    # Add libraries
    get_target_property(_library_list ChorusKit_Metadata APPLICATION_LIBRARIES)

    if(_library_list)
        foreach(_item ${_library_list})
            list(APPEND _binary_paths $<TARGET_FILE:${_item}>)
        endforeach()
    endif()

    # Add executables
    get_target_property(_executable_list ChorusKit_Metadata PLAIN_EXECUTABLES)
    set(_executable_paths)

    if(_executable_list)
        foreach(_item ${_executable_list})
            list(APPEND _executable_paths $<TARGET_FILE:${_item}>)
        endforeach()
    endif()

    # Get petool
    if(TARGET ckwindeps)
        set(_petool "$<TARGET_FILE:ckwindeps>")
    else()
        get_target_property(_petool ChorusKit::windeps LOCATION)
    endif()

    # Compute escaped path string
    set(_binary_paths_escaped "")

    foreach(_item ${_binary_paths})
        set(_binary_paths_escaped "${_binary_paths_escaped} \"${_item}\"")
    endforeach()

    # Run command
    if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL Debug)
        set(_debug --debug)
    else()
        set(_debug)
    endif()

    if(CK_RUN_SCRIPTS_VERBOSE)
        set(_verbose --verbose)
    else()
        set(_verbose)
    endif()

    if(WIN32)
        # Get library searching paths
        get_target_property(_searching_paths ChorusKit_Metadata LIBRARY_SEARCHING_PATHS)

        # Compute escaped path string 2
        set(_searching_paths_escaped "")

        foreach(_item ${_searching_paths})
            set(_searching_paths_escaped "${_searching_paths_escaped} \"${_item}\"")
        endforeach()

        add_custom_command(TARGET ChorusKit_AppLocalDeps POST_BUILD
            COMMAND ${Python_EXECUTABLE} "${CK_CMAKE_MODULES_DIR}/python/windeploy.py"
            --prefix ${CK_BUILD_MAIN_DIR}
            --qmake ${QT_QMAKE_EXECUTABLE}
            --petool ${_petool}
            --dirs ${_searching_paths}
            --files ${_binary_paths} ${_executable_paths}
            ${_debug} ${_verbose}
            COMMENT "Running post deploy script..."
            WORKING_DIRECTORY ${CK_BUILD_MAIN_DIR}
        )

        install(CODE "
            message(STATUS \"Running post deploy script...\")
            execute_process(
                COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target ChorusKit_ReleaseTranslations
                COMMAND \"${Python_EXECUTABLE}\" \"${CK_CMAKE_MODULES_DIR}/python/windeploy.py\"
                --prefix \"${CMAKE_INSTALL_PREFIX}\"
                --qmake \"${QT_QMAKE_EXECUTABLE}\"
                --petool \"${_petool}\"
                --dirs ${_searching_paths_escaped}
                --files ${_binary_paths_escaped}
                ${_debug} ${_verbose}
                WORKING_DIRECTORY \"${CK_BUILD_MAIN_DIR}\"
            )
        ")
    elseif(APPLE)
    # TODO...
    else()
        # TODO...
    endif()
endfunction()

include(${CK_CMAKE_MODULES_DIR}/modules/Basic.cmake)
include(${CK_CMAKE_MODULES_DIR}/modules/FileCopy.cmake)
include(${CK_CMAKE_MODULES_DIR}/modules/Target.cmake)
include(${CK_CMAKE_MODULES_DIR}/modules/Translate.cmake)