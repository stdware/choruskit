include_guard(DIRECTORY)

include(${CMAKE_CURRENT_LIST_DIR}/Basic.cmake)

# Required Variables: 

#[[
Add Windows resource file and manifest file.

    ck_attach_win_rc_file(<target>
        [NAME           <name>]
        [VERSION        <version>]
        [DESCRIPTION    <desc>]
        [COPYRIGHT      <copyright>]
        [ICO            <file>]
        [MANIFEST]
    )
]] #
function(ck_attach_win_rc_file _target)
    set(options MANIFEST)
    set(oneValueArgs NAME VERSION DESCRIPTION COPYRIGHT ICO)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    ck_set_value(_app_name FUNC_NAME ${_target})
    ck_set_value(_app_version FUNC_VERSION "${PROJECT_VERSION}")
    ck_set_value(_app_desc FUNC_DESCRIPTION ${_app_name})
    ck_set_value(_app_copyright FUNC_COPYRIGHT ${_target})

    ck_parse_version(_app_version ${_app_version})

    if(FUNC_ICO)
        set(RC_ICON_COMMENT)
        get_filename_component(RC_ICON_PATH ${FUNC_ICO} ABSOLUTE)
    else()
        set(RC_ICON_COMMENT "//")
        set(RC_ICON_PATH)
    endif()

    set(RC_VERSION ${_app_version_1},${_app_version_2},${_app_version_3},${_app_version_4})
    set(RC_APPLICATION_NAME ${_app_name})
    set(RC_VERSION_STRING ${_app_version})
    set(RC_DESCRIPTION ${_app_desc})
    set(RC_COPYRIGHT ${_app_copyright})

    set(_rc_path ${CMAKE_CURRENT_BINARY_DIR}/${_target}.rc)
    configure_file(${CK_CMAKE_MODULES_DIR}/res/WinResource.rc.in ${_rc_path} @ONLY)
    target_sources(${_target} PRIVATE ${_rc_path})

    if(FUNC_MANIFEST)
        set(MANIFEST_VERSION ${_app_version})
        set(MANIFEST_IDENTIFIER ${_app_name})
        set(MANIFEST_DESCRIPTION ${_app_desc})

        set(_manifest_path ${CMAKE_CURRENT_BINARY_DIR}/${_target}.manifest)
        configure_file(${CK_CMAKE_MODULES_DIR}/res/WinManifest.manifest.in ${_manifest_path} @ONLY)
        target_sources(${_target} PRIVATE ${_manifest_path})
    endif()

    if(FUNC_NAME)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${FUNC_NAME})
    endif()
endfunction()

#[[
Add Mac bundle info.

    ck_attach_mac_bundle(<target>
        [NAME           <name>]
        [VERSION        <version>]
        [DESCRIPTION    <desc>]
        [COPYRIGHT      <copyright>]
        [ICNS           <file>]
    )
]] #
function(ck_attach_mac_bundle _target)
    set(options)
    set(oneValueArgs NAME VERSION DESCRIPTION COPYRIGHT ICNS)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    ck_set_value(_app_name FUNC_NAME ${_target})
    ck_set_value(_app_version FUNC_VERSION "${PROJECT_VERSION}")
    ck_set_value(_app_desc FUNC_DESCRIPTION ${_app_name})
    ck_set_value(_app_copyright FUNC_COPYRIGHT ${_target})

    ck_parse_version(_app_version ${_app_version})

    # configure mac plist
    set_target_properties(${_target} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME ${_app_name}
        MACOSX_BUNDLE_EXECUTABLE_NAME ${_app_name}
        MACOSX_BUNDLE_INFO_STRING ${_app_desc}
        MACOSX_BUNDLE_GUI_IDENTIFIER ${_app_name}
        MACOSX_BUNDLE_BUNDLE_VERSION ${_app_version}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${_app_version_1}.${_app_version_2}
        MACOSX_BUNDLE_COPYRIGHT ${_app_copyright}
    )

    if(FUNC_ICNS)
        # And this part tells CMake where to find and install the file itself
        set_source_files_properties(${FUNC_ICNS} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )

        # NOTE: Don't include the path in MACOSX_BUNDLE_ICON_FILE -- this is
        # the property added to Info.plist
        get_filename_component(_icns_name ${FUNC_ICNS} NAME)

        # configure mac plist
        set_target_properties(${_target} PROPERTIES
            MACOSX_BUNDLE_ICON_FILE ${_icns_name}
        )

        # ICNS icon MUST be added to executable's sources list, for some reason
        # Only apple can do
        target_sources(${_target} PRIVATE ${FUNC_ICNS})
    endif()

    if(FUNC_NAME)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${FUNC_NAME})
    endif()
endfunction()

#[[
Generate Windows shortcut after building target.

    ck_create_win_shortcut(<target> <dir>
        [OUTPUT_NAME <name]
    )
]] #
function(ck_create_win_shortcut _target _dir)
    set(options)
    set(oneValueArgs OUTPUT_NAME)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    ck_set_value(_output_name FUNC_OUTPUT_NAME $<TARGET_FILE_BASE_NAME:${_target}>)

    string(RANDOM LENGTH 8 _rand)
    set(_vbs_name ${CMAKE_CURRENT_BINARY_DIR}/${_target}_shortcut.vbs)
    set(_vbs_temp ${_vbs_name}.in)

    set(_lnk_path "${_dir}/${_output_name}.lnk")

    set(SHORTCUT_PATH ${_lnk_path})
    set(SHORTCUT_TARGET_PATH $<TARGET_FILE:${_target}>)
    set(SHORTCUT_WORKING_DIRECOTRY $<TARGET_FILE_DIR:${_target}>)
    set(SHORTCUT_DESCRIPTION $<TARGET_FILE_BASE_NAME:${_target}>)
    set(SHORTCUT_ICON_LOCATION $<TARGET_FILE:${_target}>)

    configure_file(
        ${CK_CMAKE_MODULES_DIR}/res/WinCreateShortcut.vbs.in
        ${_vbs_temp}
        @ONLY
    )
    file(GENERATE OUTPUT ${_vbs_name} INPUT ${_vbs_temp})

    add_custom_command(
        TARGET ${_target} POST_BUILD
        COMMAND cscript ${_vbs_name}
        BYPRODUCTS ${_lnk_path}
    )
endfunction()