if(CHORUSKIT_BUILD_TRANSLATIONS)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS LinguistTools REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools REQUIRED)

    set(_my_ts_files)
    set(_my_qm_files)
    set(_qm_dir ${CMAKE_BINARY_DIR}/share/${CHORUSKIT_INSTALL_NAME}/translations)

    foreach(_item ${CHORUSKIT_TRANSLATIONS})
        list(APPEND _my_ts_files ${CMAKE_CURRENT_SOURCE_DIR}/translations/choruskit_${_item}.ts)
    endforeach()

    set_source_files_properties(${_my_ts_files}
        PROPERTIES OUTPUT_LOCATION ${_qm_dir}
    )

    get_property(_targets GLOBAL PROPERTY CHORUSKIT_TRANSLATE_TARGETS)
    set(_src)

    foreach(_item ${_targets})
        get_target_property(_files ${_item} SOURCES)
        list(FILTER _files INCLUDE REGEX ".+\\.(cpp|cc)")
        list(FILTER _files EXCLUDE REGEX "(qasc|moc)_.+")
        list(APPEND _src ${_files})
    endforeach()

    qt5_create_translation(_my_qm_files ${_my_ts_files} ${_src})

    add_custom_target(choruskit_lupdate_task DEPENDS ${_my_ts_files})
    add_custom_target(choruskit_lrelease_task DEPENDS ${_my_qm_files})

    if(CHORUSKIT_INSTALL)
        install(CODE "
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target choruskit_lupdate_task)
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target choruskit_lrelease_task)
        ")
        install(DIRECTORY ${_qm_dir}/
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CHORUSKIT_INSTALL_NAME}/translations
            FILES_MATCHING PATTERN "*.qm"
        )
    endif()
endif()