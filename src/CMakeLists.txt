qm_import(Preprocess)

if(NOT TARGET qtmediate::Core)
    find_package(qtmediate REQUIRED)
endif()

set(CHORUSKIT_PROJECT_DESCRIPTION "ChorusKit Framework Library")
set(CHORUSKIT_PROJECT_COPYRIGHT "Copyright 2019-2023 OpenVPI")
set(CHORUSKIT_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)
set(CHORUSKIT_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../etc/include)

# ----------------------------------
# Configurations
# ----------------------------------

# ----------------------------------
# CMake API
# ----------------------------------
function(choruskit_add_library _target _type)
    set(options AUTOGEN NO_SYNC_INCLUDE NO_WIN_RC)
    set(oneValueArgs SYNC_INCLUDE_PREFIX PREFIX)
    set(multiValueArgs SYNC_INCLUDE_OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_AUTOGEN)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
    endif()

    add_library(${_target} ${_type})

    if(WIN32 AND NOT FUNC_NO_WIN_RC AND(${_type} STREQUAL "SHARED"))
        qm_add_win_rc(${_target}
            NAME ${CHORUSKIT_INSTALL_NAME}
            DESCRIPTION ${CHORUSKIT_PROJECT_DESCRIPTION}
            COPYRIGHT ${CHORUSKIT_PROJECT_COPYRIGHT}
        )
    endif()

    if(FUNC_PREFIX)
        set(_prefix_option PREFIX ${FUNC_PREFIX})
    else()
        set(_prefix_option)
    endif()

    # Set global definitions
    qm_export_defines(${_target} ${_prefix_option})

    # Configure target
    qm_configure_target(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Add include directories
    target_include_directories(${_target} PRIVATE ${CHORUSKIT_BUILD_INCLUDE_DIR})
    target_include_directories(${_target} PRIVATE .)

    # Library name
    if(${_target} MATCHES "^Ck(.+)")
        set(_name ${CMAKE_MATCH_1})
        set_target_properties(${_target} PROPERTIES EXPORT_NAME ${_name})
    else()
        set(_name ${_target})
    endif()

    add_library(${CHORUSKIT_INSTALL_NAME}::${_name} ALIAS ${_target})

    if(FUNC_SYNC_INCLUDE_PREFIX)
        set(_inc_name ${FUNC_SYNC_INCLUDE_PREFIX})
    else()
        set(_inc_name ${_target})
    endif()

    set(_install_options)

    if(CHORUSKIT_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${CHORUSKIT_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )

        target_include_directories(${_target} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}>"
        )

        set(_install_options
            INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}/${_inc_name}"
        )
    endif()

    if(NOT FUNC_NO_SYNC_INCLUDE)
        # Generate a standard include directory in build directory
        qm_sync_include(. "${CHORUSKIT_GENERATED_INCLUDE_DIR}/${_inc_name}" ${_install_options}
            ${FUNC_SYNC_INCLUDE_OPTIONS}
        )
        target_include_directories(${_target} PUBLIC
            "$<BUILD_INTERFACE:${CHORUSKIT_GENERATED_INCLUDE_DIR}>"
        )
    endif()

    set_property(GLOBAL APPEND PROPERTY CHORUSKIT_TRANSLATE_TARGETS ${_target})
endfunction()

function(choruskit_configure_include _target _name)
    target_include_directories(${_target} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
    )

    if(CHORUSKIT_INSTALL)
        target_include_directories(${_target} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}/${_name}>"
        )
        file(GLOB _headers *.h)
        install(FILES ${_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}/${_name}")
    endif()
endfunction()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(3rdparty)

# add_subdirectory(appcore)

# add_subdirectory(loader)
add_subdirectory(svsbasic)

add_subdirectory(svswidgets)

# ----------------------------------
# Install
# ----------------------------------
if(CHORUSKIT_INSTALL)
    # Add install target
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${CHORUSKIT_INSTALL_NAME})

    # Add version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${CHORUSKIT_INSTALL_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # Add configuration file
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/${CHORUSKIT_INSTALL_NAME}Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${CHORUSKIT_INSTALL_NAME}Config.cmake"
        INSTALL_DESTINATION ${_install_dir}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install cmake files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${CHORUSKIT_INSTALL_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${CHORUSKIT_INSTALL_NAME}ConfigVersion.cmake"
        DESTINATION ${_install_dir}
    )

    # Install cmake targets files
    install(EXPORT ${CHORUSKIT_INSTALL_NAME}Targets
        FILE "${CHORUSKIT_INSTALL_NAME}Targets.cmake"
        NAMESPACE ${CHORUSKIT_INSTALL_NAME}::
        DESTINATION ${_install_dir}
    )

    # Install cmake modules
    install(DIRECTORY ${CK_CMAKE_MODULES_DIR}/
        DESTINATION ${_install_dir}/cmake
        PATTERN "__pycache__" EXCLUDE
    )
endif()