if(CHORUSKIT_BUILD_TRANSLATIONS)
    qm_import(Translate)
    qm_find_qt(LinguistTools)

    add_custom_target(choruskit_lupdate_task)
    add_custom_target(choruskit_lrelease_task)

    # choruskit
    get_property(_targets GLOBAL PROPERTY CHORUSKIT_TRANSLATE_TARGETS)
    set(_qm_dir ${CMAKE_BUILD_SHARE_DIR}/${CHORUSKIT_INSTALL_NAME}/translations)
    qm_add_translation(choruskit_translations
        LOCALES ja_JP zh_CN zh_TW
        PREFIX choruskit
        TARGETS ${_targets}
        TS_DIR translations
        QM_DIR ${_qm_dir}
        TS_DEPENDS choruskit_lupdate_task
        QM_DEPENDS choruskit_lrelease_task
    )

    if(CHORUSKIT_INSTALL)
        install(CODE "
            # execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target choruskit_lupdate_task)
            execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target choruskit_lrelease_task)
        ")
        install(DIRECTORY ${_qm_dir}/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${CHORUSKIT_INSTALL_NAME}/translations
            FILES_MATCHING PATTERN "*.qm"
        )
    endif()
endif()

if(CHORUSKIT_INSTALL)
    install(DIRECTORY doc/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/${CHORUSKIT_INSTALL_NAME}
    )
endif()