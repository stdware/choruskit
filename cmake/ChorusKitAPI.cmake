include_guard(DIRECTORY)

if(NOT DEFINED CK_CMAKE_MODULES_DIR)
    set(CK_CMAKE_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()

#[[
Initialize ChorusKitApi global settings.

    ck_init_buildsystem()
]] #
macro(ck_init_build_system _app)
    # Check platform, only Windows/Macintosh/Linux is supported
    if(APPLE)
        set(CK_PLATFORM_NAME Macintosh)
        set(CK_PLATFORM_LOWER mac)
    elseif(WIN32)
        set(CK_PLATFORM_NAME Windows)
        set(CK_PLATFORM_LOWER win)
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
        set(LINUX true CACHE BOOL "Linux System" FORCE)
        set(CK_PLATFORM_NAME Linux)
        set(CK_PLATFORM_LOWER linux)
    else()
        message(FATAL_ERROR "Unsupported System ${CMAKE_HOST_SYSTEM_NAME}!!!")
    endif()

    # Set main output directory
    if(NOT DEFINED CK_BUILD_MAIN_DIR)
        if(CMAKE_CONFIGURATION_TYPES OR NOT CMAKE_BUILD_TYPE)
            message(FATAL_ERROR "ChorusKit: multi-config is not supported.")
        endif()

        set(CK_BUILD_MAIN_DIR ${CMAKE_BINARY_DIR}/out-${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_BUILD_TYPE})
    endif()

    # Whether to build Windows Console executables
    if(NOT DEFINED CK_ENABLE_CONSOLE)
        set(CK_ENABLE_CONSOLE on)
    endif()

    # Whether to install developer files
    if(NOT DEFINED CK_ENABLE_DEVEL)
        set(CK_ENABLE_DEVEL off)
    endif()

    # Root directory
    if(NOT DEFINED CK_REPO_ROOT_DIR)
        set(CK_REPO_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
    endif()

    # Application name
    if(NOT DEFINED CK_APPLICATION_NAME)
        set(CK_APPLICATION_NAME ChorusKit)
    endif()

    # Application description
    if(NOT DEFINED CK_APPLICATION_DESCRIPTION)
        set(CK_APPLICATION_DESCRIPTION ${CK_APPLICATION_NAME})
    endif()

    # Application version
    if(NOT DEFINED CK_APPLICATION_VERSION)
        if(PROJECT_VERSION)
            set(CK_APPLICATION_VERSION ${PROJECT_VERSION})
        else()
            set(CK_APPLICATION_VERSION "0.0.0.0")
        endif()
    endif()

    # Application vendor
    if(NOT DEFINED CK_APPLICATION_VENDOR)
        set(CK_APPLICATION_VENDOR OpenVPI)
    endif()

    # Set time variables
    set(CK_DEV_START_YEAR 2019)
    string(TIMESTAMP CK_CURRENT_YEAR "%Y")

    # Initialization guard
    if(CK_INITIALIZED)
        message(FATAL_ERROR "ck_init_build_system: build system has initialized")
    endif()

    set(CK_INITIALIZED on)

    # Source directory when configuring
    set(CK_CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    # Export targets
    set(CK_INSTALL_EXPORT ${_app}Targets)

    # Set output directories
    set(CK_ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/etc)
    set(CK_BUILD_INCLUDE_DIR ${CK_ARCHIVE_OUTPUT_PATH}/include)

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
    # APPLICATION_PLUGINS
    # APPLICATION_LIBRARIES
endmacro()

#[[
Add application target.

    ck_configure_application(
        [ICO ico...]
        [ICNS icns]

        [WIN_SHORTCUT]
        [SKIP_EXPORT]
    )

    ICO:  set Windows icon file
    ICNS: set Mac icon file

    WIN_SHORTCUT: create shortcut after build
]] #
function(ck_configure_application)
    set(options WIN_SHORTCUT)
    set(oneValueArgs ICNS)
    set(multiValueArgs ICO)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_target ${CK_APPLICATION_NAME})
    add_executable(${_target})

    # Make location dependent executable, otherwise GNOME cannot recognize
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        target_link_options(${_target} PRIVATE "-no-pie")
    endif()

    # Set resource files arguments
    set(_rc_args
        NAME ${CK_APPLICATION_NAME}
        VERSION ${CK_APPLICATION_VERSION}
        DESCRIPTION ${CK_APPLICATION_DESCRIPTION}
        COPYRIGHT "Copyright ${CK_DEV_START_YEAR}-${CK_CURRENT_YEAR} ${CK_APPLICATION_VENDOR}"
    )

    if(APPLE)
        if(FUNC_ICNS)
            set(_icns ICON ${FUNC_ICNS})
        else()
            set(_icns)
        endif()

        # Add mac bundle
        qtmediate_add_mac_bundle(${_target}
            ${_rc_args}
            ${_icns}
        )

        # Set output directory
        set_target_properties(${_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_MAIN_DIR}
        )
    else()
        if(WIN32)
            if(FUNC_ICO)
                set(_ico ICONS ${FUNC_ICO})
            else()
                set(_ico)
            endif()

            # Add windows rc file
            qtmediate_add_win_rc_enhanced(${_target}
                ${_rc_args}
                ${_ico}
            )

            # Add manifest
            qtmediate_add_win_manifest(${_target}
                ${_rc_args}
                ${_ico}
            )

            # Set windows application type
            if(NOT CK_ENABLE_CONSOLE)
                set_target_properties(${_target} PROPERTIES
                    WIN32_EXECUTABLE TRUE
                )
            endif()

            # Add shortcut
            if(FUNC_WIN_SHORTCUT)
                qtmediate_create_win_shortcut(${_target} ${CK_BUILD_MAIN_DIR}
                    OUTPUT_NAME ${_target}
                )
            endif()
        endif()

        set_target_properties(${_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_RUNTIME_DIR}
        )
    endif()

    # Setup install commands
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

    ck_add_plugin(<target>
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
function(ck_add_plugin _target)
    set(options SKIP_EXPORT)
    set(oneValueArgs VENDOR PLUGIN_JSON)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Add Qt Moc
    _ck_set_cmake_autoxxx(on)

    # Add library target and attach definitions
    _ck_add_library_internal(${_target} SHARED ${FUNC_UNPARSED_ARGUMENTS})
    add_library(${CK_APPLICATION_NAME}::${_target} ALIAS ${_target})

    # Add target level dependency
    add_dependencies(${CK_APPLICATION_NAME} ${_target})

    qtmediate_set_value(_vendor FUNC_VENDOR ${CK_APPLICATION_VENDOR})

    if(WIN32)
        set(_copyright "Copyright ${CK_DEV_START_YEAR}-${CK_CURRENT_YEAR} ${_vendor}")

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
    set(PLUGIN_METADATA_YEAR "${CK_CURRENT_YEAR}")

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
    set(_shared)
    _ck_check_shared_library(${_target} _shared)

    if(CK_INITIALIZED)
        add_library(${CK_APPLICATION_NAME}::${_target} ALIAS ${_target})
    endif()

    # Add windows rc file
    if(WIN32)
        if(FUNC_COPYRIGHT)
            set(_copyright ${FUNC_COPYRIGHT})
        else()
            qtmediate_set_value(_vendor FUNC_VENDOR "${CK_APPLICATION_VENDOR}")
            set(_copyright "Copyright ${CK_DEV_START_YEAR}-${CK_CURRENT_YEAR} ${_vendor}")
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

    if(APPLE)
        set_target_properties(${_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_MAIN_DIR}
        )
    else()
        set_target_properties(${_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_RUNTIME_DIR}
        )
    endif()

    set_property(TARGET ChorusKit_Metadata APPEND PROPERTY PLAIN_EXECUTABLES ${_target})
endfunction()

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
    set(_resilt)
    qtmediate_parse_copy_args("${FUNC_UNPARSED_ARGUMENTS}" _result _error)

    if(_error)
        message(FATAL_ERROR "ck_add_attached_files: ${_error}")
    endif()

    foreach(_src ${_result})
        list(POP_BACK _src _dest)

        if(NOT FUNC_SKIP_BUILD)
            qtmediate_add_copy_command(${_target}
                SOURCES ${_src}
                DESTINATION ${_dest}
            )
        endif()

        if(NOT FUNC_SKIP_INSTALL)
            _ck_install_resources(_src ${_dest} "$<TARGET_FILE_DIR:${_target}>")
        endif()
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
    set(options TARGET SKIP_BUILD SKIP_INSTALL)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_error)
    set(_resilt)
    qtmediate_parse_copy_args("${FUNC_UNPARSED_ARGUMENTS}" _result _error)

    if(_error)
        message(FATAL_ERROR "ck_add_shared_files: ${_error}")
    endif()

    if(FUNC_TARGET)
        set(_target ${FUNC_TARGET})
    else()
        string(RANDOM LENGTH 8 _rand)
        set(_target shared_copy_command_${_rand})
    endif()

    if(TARGET ChorusKit_CopySharedFiles)
        add_dependencies(ChorusKit_CopySharedFiles ${_target})
    endif()

    foreach(_src ${_result})
        list(POP_BACK _src _dest)

        # Determine destination
        qtmediate_has_genex(_has_genex ${_dest})

        if(NOT _has_genex AND NOT IS_ABSOLUTE ${_dest})
            set(_dest "${CK_BUILD_SHARE_DIR}/${FUNC_DESTINATION}")
        endif()

        if(NOT FUNC_SKIP_BUILD)
            qtmediate_add_copy_command(${_target}
                SOURCES ${_src}
                DESTINATION ${_dest}
            )
        endif()

        if(NOT FUNC_SKIP_INSTALL)
            _ck_install_resources(_src ${_dest} off)
        endif()
    endforeach()
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

function(_ck_install_resources _src_list _dest _relative_fallback_path)
    set(_src_quoted)

    foreach(_item IN LISTS ${_src_list})
        set(_src_quoted "${_src_quoted}\"${_item}\" ")
    endforeach()

    set(_relative_fallback)

    if(_relative_fallback_path)
        set(_relative_fallback "
            if(NOT IS_ABSOLUTE \${_dest})
                file(RELATIVE_PATH _rel_path \"${CK_BUILD_MAIN_DIR}\" \"${_relative_fallback_path}/\${_dest}\")
                set(_dest \"\${CMAKE_INSTALL_PREFIX}/\${_rel_path}\")
            endif()
        ")
    endif()

    install(CODE "
        set(_src ${_src_quoted})
        set(_dest \"${_dest}\")

        ${_relative_fallback}

        foreach(_file \${_src})
            get_filename_component(_path \${_file} ABSOLUTE BASE_DIR \"${CMAKE_CURRENT_SOURCE_DIR}\")

            if(IS_DIRECTORY \${_path})
                set(_type DIRECTORY)
            else()
                set(_type FILE)
            endif()

            file(INSTALL DESTINATION \"\${_dest}\"
                TYPE \${_type}
                FILES \${_path}
            )
        endforeach()
    ")
endfunction()
