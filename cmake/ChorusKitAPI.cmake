include_guard(DIRECTORY)

if(NOT TARGET qmsetup::library)
    find_package(qmsetup CONFIG REQUIRED)
endif()

qm_import(Filesystem Preprocess Deploy)

if(NOT DEFINED CK_CMAKE_MODULES_DIR)
    set(CK_CMAKE_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()

#[[
    Initialize ChorusKitApi global configuration.

    ck_init_buildsystem()

    Customizable Variables:
        CK_APPLICATION_NAME
        CK_APPLICATION_DESCRIPTION
        CK_APPLICATION_VERSION
        CK_APPLICATION_VENDOR
        
        CK_CMAKE_SOURCE_DIR (not suggested)
        CK_REPO_ROOT_DIR

        CK_DEV_START_YEAR
        CK_BUILDINFO_PREFIX
        CK_CONFIG_HEADER_FILENAME
        CK_BUILDINFO_HEADER_FILENAME

        CK_ENABLE_CONSOLE
        CK_ENABLE_INSTALL
        CK_ENABLE_DEVEL
        CK_WIN_APPLOCAL_DEPS
        CK_SYNC_INCLUDE_FORCE

]] #
macro(ck_init_buildsystem)
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
    qm_init_directories()
    set(CK_BUILD_MAIN_DIR ${QMSETUP_BUILD_DIR})

    # Whether to build Windows Console executables
    if(NOT DEFINED CK_ENABLE_CONSOLE)
        set(CK_ENABLE_CONSOLE on)
    endif()

    # Install or not
    if(NOT DEFINED CK_ENABLE_INSTALL)
        set(CK_ENABLE_INSTALL on)
    endif()

    if(CK_ENABLE_INSTALL)
        include(GNUInstallDirs)
        include(CMakePackageConfigHelpers)
    endif()

    # Whether to install developer files
    if(NOT DEFINED CK_ENABLE_DEVEL)
        set(CK_ENABLE_DEVEL off)
    endif()

    # Whether to set FORCE option to sync_include
    if(NOT DEFINED CK_SYNC_INCLUDE_FORCE)
        set(CK_SYNC_INCLUDE_FORCE off)
    endif()

    # Root directory
    if(NOT DEFINED CK_REPO_ROOT_DIR)
        set(CK_REPO_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
    endif()

    # Application name
    if(NOT DEFINED CK_APPLICATION_NAME)
        set(CK_APPLICATION_NAME Application)
    endif()

    # Application description
    if(NOT DEFINED CK_APPLICATION_DESCRIPTION)
        set(CK_APPLICATION_DESCRIPTION ${CK_APPLICATION_NAME})
    endif()

    # Application version
    if(NOT DEFINED CK_APPLICATION_VERSION)
        set(CK_APPLICATION_VERSION)
        qm_set_value(CK_APPLICATION_VERSION PROJECT_VERSION "0.0.0.0")
    endif()

    # Application vendor
    if(NOT DEFINED CK_APPLICATION_VENDOR)
        set(CK_APPLICATION_VENDOR unknown)
    endif()

    # Set time variables
    if(NOT DEFINED CK_DEV_START_YEAR)
        set(CK_DEV_START_YEAR 2019)
    endif()

    string(TIMESTAMP CK_CURRENT_YEAR "%Y")

    # Set configure variables
    if(NOT DEFINED CK_BUILDINFO_PREFIX)
        set(CK_BUILDINFO_PREFIX ${CK_APPLICATION_NAME})
    endif()

    if(NOT DEFINED CK_CONFIG_HEADER_FILENAME)
        set(CK_CONFIG_HEADER_FILENAME "choruskit_config.h")
    endif()

    if(NOT DEFINED CK_BUILDINFO_HEADER_FILENAME)
        set(CK_BUILDINFO_HEADER_FILENAME "choruskit_buildinfo.h")
    endif()

    # Set windows dependencies deploy variables
    if(NOT DEFINED CK_WIN_APPLOCAL_DEPS)
        set(CK_WIN_APPLOCAL_DEPS off)
    endif()

    # Initialization guard
    if(CK_INITIALIZED)
        message(FATAL_ERROR "ck_init_build_system: build system has initialized")
    endif()

    set(CK_INITIALIZED on)

    # Source directory when configuring
    if(NOT DEFINED CK_CMAKE_SOURCE_DIR)
        set(CK_CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    # Export targets
    set(CK_INSTALL_EXPORT ${CK_APPLICATION_NAME}Targets)

    # Set output directories
    set(CK_ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/etc)
    set(CK_BUILD_INCLUDE_DIR ${CK_ARCHIVE_OUTPUT_PATH}/include)
    set(CK_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)

    if(APPLE)
        set(_CK_BUILD_BASE_DIR ${CK_BUILD_MAIN_DIR}/${CK_APPLICATION_NAME}.app/Contents)
        set(CK_BUILD_RUNTIME_DIR ${_CK_BUILD_BASE_DIR}/MacOS)

        # When CMake installs Mac bundle application, it simply copies the bundle directory in
        # build directory to the install destination, as a result, some build phase files may
        # be mistakenly installed because the release doesn't need them.
        set(CK_BUILD_LIBRARY_DIR ${CK_BUILD_MAIN_DIR}/lib)

        set(CK_BUILD_PLUGINS_DIR ${_CK_BUILD_BASE_DIR}/Plugins)
        set(CK_BUILD_SHARE_DIR ${_CK_BUILD_BASE_DIR}/Resources)
        set(CK_BUILD_DATA_DIR ${CK_BUILD_SHARE_DIR})
        set(CK_BUILD_DOC_DIR ${CK_BUILD_SHARE_DIR}/doc)
        set(CK_BUILD_QT_CONF_DIR ${_CK_BUILD_BASE_DIR}/Resources)

        set(_CK_INSTALL_BASE_DIR ${CK_APPLICATION_NAME}.app/Contents)
        set(CK_INSTALL_RUNTIME_DIR ${_CK_INSTALL_BASE_DIR}/MacOS)
        set(CK_INSTALL_LIBRARY_DIR ${_CK_INSTALL_BASE_DIR}/Frameworks)
        set(CK_INSTALL_PLUGINS_DIR ${_CK_INSTALL_BASE_DIR}/Plugins)
        set(CK_INSTALL_SHARE_DIR ${_CK_INSTALL_BASE_DIR}/Resources/share)
        set(CK_INSTALL_DATA_DIR ${_CK_INSTALL_BASE_DIR}/Resources)
        set(CK_INSTALL_DOC_DIR ${CK_INSTALL_SHARE_DIR}/doc)
        set(CK_INSTALL_INCLUDE_DIR ${_CK_INSTALL_BASE_DIR}/Resources/include/${CK_APPLICATION_NAME})
        set(CK_INSTALL_CMAKE_DIR ${_CK_INSTALL_BASE_DIR}/Resources/lib/cmake/${CK_APPLICATION_NAME})
        set(CK_INSTALL_QML_DIR ${CK_INSTALL_DATA_DIR}/qml)
    else()
        set(CK_BUILD_RUNTIME_DIR ${CK_BUILD_MAIN_DIR}/bin)
        set(CK_BUILD_LIBRARY_DIR ${CK_BUILD_MAIN_DIR}/lib)
        set(CK_BUILD_PLUGINS_DIR ${CK_BUILD_MAIN_DIR}/lib/${CK_APPLICATION_NAME}/plugins)
        set(CK_BUILD_SHARE_DIR ${CK_BUILD_MAIN_DIR}/share)
        set(CK_BUILD_DATA_DIR ${CK_BUILD_SHARE_DIR}/${CK_APPLICATION_NAME})
        set(CK_BUILD_DOC_DIR ${CK_BUILD_SHARE_DIR}/doc/${CK_APPLICATION_NAME})
        set(CK_BUILD_QT_CONF_DIR ${CK_BUILD_MAIN_DIR}/bin)

        set(CK_INSTALL_RUNTIME_DIR bin)
        set(CK_INSTALL_LIBRARY_DIR lib)
        set(CK_INSTALL_PLUGINS_DIR lib/${CK_APPLICATION_NAME}/plugins)
        set(CK_INSTALL_SHARE_DIR share)
        set(CK_INSTALL_DATA_DIR ${CK_INSTALL_SHARE_DIR}/${CK_APPLICATION_NAME})
        set(CK_INSTALL_DOC_DIR ${CK_INSTALL_SHARE_DIR}/doc/${CK_APPLICATION_NAME})
        set(CK_INSTALL_INCLUDE_DIR include/${CK_APPLICATION_NAME})
        set(CK_INSTALL_CMAKE_DIR lib/cmake/${CK_APPLICATION_NAME})
        set(CK_INSTALL_QML_DIR qml)
    endif()

    # Set definition configuration
    set(QMSETUP_DEFINITION_SCOPE DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

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
    Finish ChorusKitApi global configuration.

    ck_finish_buildsystem()
#]]
macro(ck_finish_buildsystem)
    set(_ck_config_file ${CK_BUILD_INCLUDE_DIR}/${CK_CONFIG_HEADER_FILENAME})
    set(_ck_buildinfo_file ${CK_BUILD_INCLUDE_DIR}/${CK_BUILDINFO_HEADER_FILENAME})

    qm_generate_config(${_ck_config_file})
    qm_generate_build_info(${_ck_buildinfo_file} YEAR TIME PREFIX ${CK_BUILDINFO_PREFIX})

    if(CK_ENABLE_INSTALL AND CK_ENABLE_DEVEL)
        if(EXISTS ${_ck_config_file})
            install(FILES ${_ck_config_file}
                DESTINATION ${CK_INSTALL_INCLUDE_DIR}
            )
        endif()
    endif()
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
    if(LINUX)
        target_link_options(${_target} PRIVATE "-no-pie")
    elseif(WIN32)
        if(CK_WIN_APPLOCAL_DEPS)
            qm_win_applocal_deps(${_target} OUTPUT_DIR ${CK_BUILD_RUNTIME_DIR})
        endif()
    endif()

    target_include_directories(${_target} PUBLIC
        $<BUILD_INTERFACE:${CK_BUILD_INCLUDE_DIR}>
    )

    if(CK_ENABLE_INSTALL)
        target_include_directories(${_target} PUBLIC
            $<INSTALL_INTERFACE:${CK_INSTALL_INCLUDE_DIR}>
        )
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
        qm_add_mac_bundle(${_target}
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
            qm_add_win_rc_enhanced(${_target}
                ${_rc_args}
                ${_ico}
            )

            # Add manifest
            qm_add_win_manifest(${_target}
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
                qm_create_win_shortcut(${_target} ${CK_BUILD_MAIN_DIR}
                    OUTPUT_NAME ${_target}
                )
            endif()
        endif()

        set_target_properties(${_target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CK_BUILD_RUNTIME_DIR}
        )
    endif()

    if(CK_ENABLE_INSTALL)
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
        [DISPLAY_NAME   display_name]
        [VERSION        version] 
        [DESCRIPTION    desc]
        [COPYRIGHT      copyright]
        [VENDOR         vendor]
        [MACRO_PREFIX   prefix]
        [STATIC_MACRO   macro]
        [LIBRARY_MACRO  macro]
    )

    NO_PLUGIN_JSON: skip configuring the plugin.json.in
    CATEGORY: set the sub-directory name for plugin to output, which is same as `PROJECT_NAME` by default
    PLUGIN_JSON: set the custom plugin.json.in file to configure, otherwise configure the plugin.json.in in currect directory
]] #
function(ck_add_plugin _target)
    set(options SKIP_EXPORT)
    set(oneValueArgs COPYRIGHT VENDOR PLUGIN_JSON)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Add Qt Moc
    _ck_set_cmake_autoxxx(on)

    # Add library target and attach definitions
    set(_vcpkg_applocal_deps off)

    if(DEFINED VCPKG_APPLOCAL_DEPS AND VCPKG_APPLOCAL_DEPS)
        set(_vcpkg_applocal_deps on)
        set(VCPKG_APPLOCAL_DEPS off) # Temporarily disable VCPKG_APPLOCAL_DEPS
    endif()

    _ck_add_library_internal(${_target} SHARED ${FUNC_UNPARSED_ARGUMENTS})
    add_library(${CK_APPLICATION_NAME}::${_target} ALIAS ${_target})

    if(_vcpkg_applocal_deps)
        set(VCPKG_APPLOCAL_DEPS on)
    endif()

    # Add target level dependency
    add_dependencies(${CK_APPLICATION_NAME} ${_target})

    qm_set_value(_vendor FUNC_VENDOR ${CK_APPLICATION_VENDOR})
    qm_set_value(_copyright FUNC_COPYRIGHT "Copyright ${CK_DEV_START_YEAR}-${CK_CURRENT_YEAR} ${_vendor}")

    if(WIN32)
        # Add windows rc file
        qm_add_win_rc(${_target}
            COPYRIGHT "${_copyright}"
            ${FUNC_UNPARSED_ARGUMENTS}
        )
    endif()

    # Configure plugin json if specified
    if(FUNC_PLUGIN_JSON)
        ck_configure_plugin_metadata(${_target} ${FUNC_PLUGIN_JSON} ${FUNC_UNPARSED_ARGUMENTS}
            VENDOR ${_vendor}
            COPYRIGHT ${_copyright}
        )
    elseif(NOT FUNC_NO_PLUGIN_JSON AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json.in)
        ck_configure_plugin_metadata(${_target} plugin.json.in ${FUNC_UNPARSED_ARGUMENTS}
            VENDOR ${_vendor}
            COPYRIGHT ${_copyright}
        )
    endif()

    # Configure plugin desc file
    set(_tmp_desc_file ${CMAKE_CURRENT_BINARY_DIR}/${_target}Metadata/plugin.json)
    _ck_configure_plugin_desc(${_tmp_desc_file} ${FUNC_UNPARSED_ARGUMENTS})
    ck_add_attached_files(${_target}
        SRC ${_tmp_desc_file} DEST .
    )

    qm_set_value(_category FUNC_CATEGORY ${_target})

    set(_build_output_dir ${CK_BUILD_PLUGINS_DIR}/${_category})
    set(_install_output_dir ${CK_INSTALL_PLUGINS_DIR}/${_category})

    # Set output directories
    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${_build_output_dir}
        LIBRARY_OUTPUT_DIRECTORY ${_build_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY ${_build_output_dir}
    )

    if(CK_ENABLE_INSTALL)
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
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                LIBRARY DESTINATION ${_install_output_dir}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                ARCHIVE DESTINATION ${_install_output_dir}
            )
        else()
            install(TARGETS ${_target}
                ${_export}
                RUNTIME DESTINATION ${_install_output_dir}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                LIBRARY DESTINATION ${_install_output_dir}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            )
        endif()
    endif()

    set_property(TARGET ChorusKit_Metadata APPEND PROPERTY APPLICATION_PLUGINS ${_target})
endfunction()

#[[
    Configure plugin metadata json.

    ck_configure_plugin_metadata(<target>
        [NAME               name            ] 
        [DISPLAY_NAME       display_name    ]
        [VERSION            version         ] 
        [COMPAT_VERSION     compat_version  ]
        [COPYRIGHT          copyright       ]
        [VENDOR             vendor          ]
    )

    NAME: to be removed
    COMPAT_VERSION: set the PLUGIN_METADATA_COMPAT_VERSION variable as compatible version
    VERSION: set the PLUGIN_METADATA_VERSION variable as version
    VENDOR: set the PLUGIN_METADATA_VENDOR variable as vendor
]] #
function(ck_configure_plugin_metadata _target _plugin_json)
    set(options)
    set(oneValueArgs NAME DISPLAY_NAME VERSION COMPAT_VERSION COPYRIGHT VENDOR)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Set plugin metadata
    qm_set_value(_name FUNC_NAME ${PROJECT_NAME})
    qm_set_value(_display_name FUNC_DISPLAY_NAME ${_name})
    qm_set_value(_version FUNC_VERSION ${PROJECT_VERSION})
    qm_set_value(_compat_version FUNC_COMPAT_VERSION "0.0.0.0")
    qm_set_value(_vendor FUNC_VENDOR "${CK_APPLICATION_VENDOR}")

    # Fix version
    qm_parse_version(_ver ${_version})
    set(PLUGIN_METADATA_VERSION ${_ver_1}.${_ver_2}.${_ver_3}_${_ver_4})

    qm_parse_version(_compat ${_compat_version})
    set(PLUGIN_METADATA_COMPAT_VERSION ${_compat_1}.${_compat_2}.${_compat_3}_${_compat_4})

    # Get year
    set(PLUGIN_METADATA_YEAR "${CK_CURRENT_YEAR}")

    # Set other variables
    set(PLUGIN_METADATA_NAME ${_name})
    set(PLUGIN_METADATA_DISPLAY_NAME ${_display_name})
    set(PLUGIN_METADATA_VENDOR ${_vendor})

    qm_set_value(_copyright FUNC_COPYRIGHT "Copyright (C) ${PLUGIN_METADATA_YEAR} ${CK_APPLICATION_VENDOR}")
    set(PLUGIN_METADATA_COPYRIGHT ${_copyright})

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
        [STATIC_MACRO   macro]
        [LIBRARY_MACRO  macro]
    )

    SHARED: build shared library, otherwise build static library if not set
    AUTOGEN: set CMAKE_AUTOMOC, CMAKE_AUTOUIC, CMAKE_AUTORCC
    SKIP_INSTALL: skip install the target
    SKIP_EXPORT: skip export the target to installed cmake package
    NAME: set output file name and name property in Windows RC, which is same as target name by default
    VERSION: set version property in Windows RC, which is same as `PROJECT_VERSION` by default
    DESCRIPTION: set description property in Windows RC, which is same as target name by default
    VENDOR: set vendor with default copyright string in Windows RC
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
            qm_set_value(_vendor FUNC_VENDOR "${CK_APPLICATION_VENDOR}")
            set(_copyright "Copyright ${CK_DEV_START_YEAR}-${CK_CURRENT_YEAR} ${_vendor}")
        endif()

        get_target_property(_type ${_target} TYPE)

        if(_shared)
            qm_add_win_rc(${_target}
                COPYRIGHT "${_copyright}"
                ${FUNC_UNPARSED_ARGUMENTS}
            )
        endif()
    endif()

    set(_build_output_dir ${CK_BUILD_LIBRARY_DIR})
    set(_install_output_dir ${CK_INSTALL_LIBRARY_DIR})

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

    if(NOT FUNC_SKIP_INSTALL AND CK_ENABLE_INSTALL)
        if(CK_ENABLE_DEVEL)
            install(TARGETS ${_target}
                ${_export}
                RUNTIME DESTINATION ${CK_INSTALL_RUNTIME_DIR}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                LIBRARY DESTINATION ${_install_output_dir}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                ARCHIVE DESTINATION ${_install_output_dir}
            )
        elseif(_shared)
            install(TARGETS ${_target}
                ${_export}
                RUNTIME DESTINATION ${CK_INSTALL_RUNTIME_DIR}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                LIBRARY DESTINATION ${_install_output_dir}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
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

    if(WIN32)
        if(NOT FUNC_CONSOLE)
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

        if(CK_WIN_APPLOCAL_DEPS)
            qm_win_applocal_deps(${_target} OUTPUT_DIR ${CK_BUILD_RUNTIME_DIR})
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
    Sync include files for library or plugin target.

    ck_sync_include(<target>
        [DIRECTORY <dir>]
        [PREFIX <prefix>]
        [OPTIONS <options...>]
        [SKIP_INSTALL]
    )
]] #
function(ck_sync_include _target)
    set(options SKIP_INSTALL)
    set(oneValueArgs DIRECTORY PREFIX)
    set(multiValueArgs OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_inc_name)
    qm_set_value(_inc_name FUNC_PREFIX ${_target})

    set(_dir)
    qm_set_value(_dir FUNC_DIRECTORY .)

    set(_sync_options)

    set(_scope PUBLIC)
    get_target_property(_type ${_target} TYPE)

    if(_type STREQUAL "INTERFACE_LIBRARY")
        set(_scope INTERFACE)
    else()
        target_include_directories(${_target} PRIVATE ${_dir})
    endif()

    target_include_directories(${_target} ${_scope}
        "$<BUILD_INTERFACE:${CK_GENERATED_INCLUDE_DIR}>"
        "$<BUILD_INTERFACE:${CK_GENERATED_INCLUDE_DIR}/${_inc_name}>"
    )

    if(CK_ENABLE_INSTALL AND CK_ENABLE_DEVEL AND NOT FUNC_SKIP_INSTALL)
        target_include_directories(${_target} ${_scope}
            "$<INSTALL_INTERFACE:${CK_INSTALL_INCLUDE_DIR}>"
        )

        set(_sync_options
            INSTALL_DIR "${CK_INSTALL_INCLUDE_DIR}/${_inc_name}"
        )
    endif()

    if(CK_SYNC_INCLUDE_FORCE)
        list(APPEND _sync_options FORCE)
    endif()

    # Generate a standard include directory in build directory
    qm_sync_include(${_dir} "${CK_GENERATED_INCLUDE_DIR}/${_inc_name}" ${_sync_options}
        ${FUNC_OPTIONS}
    )
endfunction()

#[[
Add a resources copying command after building the target.

    ck_add_attached_files(<target>
        [SKIP_BUILD] [SKIP_INSTALL] [VERBOSE]

        SRC <files1...> DEST <dir1>
        SRC <files2...> DEST <dir2> ...
    )
    
    SRC: source files or directories, use "*" to collect all items in directory
    DEST: destination directory, can be a generator expression
]] #
function(ck_add_attached_files _target)
    set(options SKIP_BUILD SKIP_INSTALL VERBOSE)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_error)
    set(_resilt)
    _ck_parse_copy_args("${FUNC_UNPARSED_ARGUMENTS}" _result _error)

    if(_error)
        message(FATAL_ERROR "ck_add_attached_files: ${_error}")
    endif()

    set(_options)

    if(FUNC_SKIP_BUILD)
        list(APPEND _options SKIP_BUILD)
    endif()

    if(FUNC_SKIP_INSTALL OR NOT CK_ENABLE_INSTALL)
        list(APPEND _options SKIP_INSTALL)
    else()
        list(APPEND _options INSTALL_DIR .)
    endif()

    if(FUNC_VERBOSE)
        list(APPEND _options VERBOSE)
    endif()

    foreach(_src IN LISTS _result)
        list(POP_BACK _src _dest)

        qm_add_copy_command(${_target}
            SOURCES ${_src}
            DESTINATION ${_dest}
            ${_options}
        )
    endforeach()
endfunction()

#[[
Add a resources copying command for whole project.

    ck_add_shared_files(
        [SKIP_BUILD] [SKIP_INSTALL] [VERBOSE]

        SRC <files1...> DEST <dir1>
        SRC <files2...> DEST <dir2> ...
    )

    SRC: source files or directories, use "*" to collect all items in directory
    DEST: destination directory, can be a generator expression
    
    Related Targets:
        ChorusKit_CopySharedFiles
]] #
function(ck_add_shared_files)
    set(options TARGET SKIP_BUILD SKIP_INSTALL VERBOSE)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_error)
    set(_resilt)
    _ck_parse_copy_args("${FUNC_UNPARSED_ARGUMENTS}" _result _error)

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

    set(_options)

    if(FUNC_SKIP_BUILD)
        list(APPEND _options SKIP_BUILD)
    endif()

    if(FUNC_SKIP_INSTALL)
        list(APPEND _options SKIP_INSTALL)
    else()
        list(APPEND _options INSTALL_DIR .)
    endif()

    if(FUNC_VERBOSE)
        list(APPEND _options VERBOSE)
    endif()

    foreach(_src IN LISTS _result)
        list(POP_BACK _src _dest)

        qm_add_copy_command(${_target}
            SOURCES ${_src}
            DESTINATION ${_dest}
            ${_options}
        )
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
    set(options SHARED INTERFACE)
    set(oneValueArgs NAME MACRO_PREFIX LIBRARY_MACRO STATIC_MACRO)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_options)

    if(FUNC_MACRO_PREFIX)
        set(_prefix ${FUNC_MACRO_PREFIX})
    elseif(FUNC_NAME)
        string(TOUPPER ${FUNC_NAME} _prefix)
    else()
        string(TOUPPER ${_target} _prefix)
    endif()

    list(APPEND _options PREFIX ${_prefix})

    if(FUNC_LIBRARY_MACRO)
        list(APPEND _options LIBRARY ${FUNC_LIBRARY_MACRO})
    endif()

    if(FUNC_STATIC_MACRO)
        list(APPEND _options STATIC ${FUNC_STATIC_MACRO})
    endif()

    set(_scope PUBLIC)

    if(FUNC_SHARED)
        add_library(${_target} SHARED)

        if(WIN32)
            if(CK_WIN_APPLOCAL_DEPS)
                qm_win_applocal_deps(${_target} OUTPUT_DIR ${CK_BUILD_RUNTIME_DIR})
            endif()
        endif()
    elseif(FUNC_INTERFACE)
        set(_scope INTERFACE)
        add_library(${_target} INTERFACE)
    else()
        add_library(${_target} STATIC)
    endif()

    if(NOT FUNC_INTERFACE)
        if(FUNC_NAME)
            set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${FUNC_NAME})
        endif()

        qm_export_defines(${_target} ${_options})
    endif()

    target_include_directories(${_target} ${_scope}
        $<BUILD_INTERFACE:${CK_BUILD_INCLUDE_DIR}>
    )

    if(CK_ENABLE_INSTALL)
        target_include_directories(${_target} ${_scope}
            $<INSTALL_INTERFACE:${CK_INSTALL_INCLUDE_DIR}>
        )
    endif()
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

    qm_set_value(_name FUNC_NAME ${PROJECT_NAME})

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

    # file(GENERATE OUTPUT ${_file} CONTENT ${_content})
    file(WRITE ${_file} ${_content})
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

#[[
    _ck_parse_copy_args(<args> <RESULT> <ERROR>)

    args:   SRC <files...> DEST <dir1>
            SRC <files...> DEST <dir2> ...
]] #
function(_ck_parse_copy_args _args _result _error)
    # State Machine
    set(_src)
    set(_dest)
    set(_status NONE) # NONE, SRC, DEST
    set(_count 0)

    set(_list)

    foreach(_item IN LISTS _args)
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
                set(_slash off)

                if(${_item} MATCHES "(.+)/\\**$")
                    set(_slash on)
                    set(_item ${CMAKE_MATCH_1})
                endif()

                get_filename_component(_path ${_item} ABSOLUTE)

                if(_slash)
                    set(_path "${_path}/")
                endif()

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