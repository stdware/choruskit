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
    set(options NO_SYNC_INCLUDE)
    set(oneValueArgs INCLUDE_PREFIX PREFIX)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_library(${_target} ${_type})

    if(WIN32 AND ${_type} STREQUAL "SHARED")
        qtmediate_add_win_rc(${_target}
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
    qtmediate_export_defines(${_target} ${_prefix_option})

    # Configure target
    qtmediate_configure_target(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Add include directories
    target_include_directories(${_target} PRIVATE ${CHORUSKIT_BUILD_INCLUDE_DIR})

    # Library name
    string(LENGTH "${_target}" _stringLength)
    math(EXPR _startIndex "2")
    math(EXPR _length "${_stringLength} - ${_startIndex}")
    string(SUBSTRING "${_target}" ${_startIndex} ${_length} _name)
    add_library(ChorusKit::${_name} ALIAS ${_target})
    set_target_properties(${_target} PROPERTIES EXPORT_NAME ${_name})

    if(FUNC_INCLUDE_PREFIX)
        set(_inc_name ${FUNC_INCLUDE_PREFIX})
    else()
        set(_inc_name ${_target})
    endif()

    set(_gen_include_options)

    if(CHORUSKIT_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${CHORUSKIT_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )

        target_include_directories(${PROJECT_NAME} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}>"
        )

        set(_gen_include_options INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}/${_inc_name}")
    endif()

    if(NOT FUNC_NO_SYNC_INCLUDE)
        # Generate a standard include directory in build directory
        qtmediate_sync_include(. "${CHORUSKIT_GENERATED_INCLUDE_DIR}/${_target}" CLEAN ${_gen_include_options})
        target_include_directories(${PROJECT_NAME} PUBLIC
            "$<BUILD_INTERFACE:${CHORUSKIT_GENERATED_INCLUDE_DIR}>"
        )
    endif()

    set_property(GLOBAL APPEND PROPERTY CHORUSKIT_TRANSLATE_TARGETS ${_target})
endfunction()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(3rdparty)

add_subdirectory(appcore)

add_subdirectory(loader)

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
        NAMESPACE ChorusKit::
        DESTINATION ${_install_dir}
    )

    # Install cmake modules
    install(DIRECTORY ${CK_CMAKE_MODULES_DIR}/
        DESTINATION ${_install_dir}/cmake
        PATTERN "__pycache__" EXCLUDE
    )
endif()