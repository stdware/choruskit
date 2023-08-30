include(../cmake/modules/Basic.cmake)
include(../cmake/modules/OSResource.cmake)

function(choruskit_add_library _target _type)
    add_library(${_target} ${_type} ${ARGN})

    string(REGEX REPLACE "^Ck" "" _suffix "${_target}")
    add_library(ChorusKit::${_suffix} ALIAS ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME ${_suffix})

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
    endif()

    set_property(GLOBAL APPEND PROPERTY CHORUSKIT_TRANSLATE_TARGETS ${_target})
endfunction()

function(choruskit_include_directories _target)
    foreach(_dir ${ARGN})
        get_filename_component(_abs_dir ${_dir} ABSOLUTE)
        file(RELATIVE_PATH _rel_path ${CHORUSKIT_SOURCE_DIR} ${_abs_dir})
        target_include_directories(${_target} PUBLIC
            "$<BUILD_INTERFACE:${_abs_dir}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}/${_rel_path}>"
        )
    endforeach()
endfunction()

macro(choruskit_install_headers _dir)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs EXCLUDE_DIRS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(CHORUSKIT_INSTALL)
        get_filename_component(_abs_dir ${_dir} ABSOLUTE)
        file(RELATIVE_PATH _rel_path ${CHORUSKIT_SOURCE_DIR} ${_abs_dir})
        set(_dest ${CMAKE_INSTALL_INCLUDEDIR}/${CHORUSKIT_INSTALL_NAME}/${_rel_path})

        set(_exclude)

        if(FUNC_EXCLUDE_DIRS)
            foreach(_item ${FUNC_EXCLUDE_DIRS})
                list(APPEND _exclude PATTERN "${_item}/*" EXCLUDE)
            endforeach()
        endif()

        install(DIRECTORY ${_abs_dir}/
            DESTINATION ${_dest}
            FILES_MATCHING PATTERN "*.h"
            ${_exclude}
        )

        # Remove empty directories
        install(CODE "
            execute_process(
                COMMAND \"${CMAKE_COMMAND}\"
                -D \"dir=${CMAKE_INSTALL_PREFIX}/${_dest}\"
                -P \"${CK_CMAKE_MODULES_DIR}/commands/RemoveEmptyDirs.cmake\"
                COMMAND_ERROR_IS_FATAL ANY
            )
        ")
    endif()
endmacro()

add_subdirectory(3rdparty)

if(NOT TARGET QMCore)
    find_package(qtmediate REQUIRED)
endif()

add_subdirectory(appcore)

add_subdirectory(loader)

add_subdirectory(svsbasic)

add_subdirectory(svswidgets)

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