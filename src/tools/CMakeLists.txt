function(choruskit_install_tool _target)
    # Vcpkg requires the binary to be installed in "tools/${PORT}" directory
    if(QMSETUP_VCPKG_TOOLS_HINT)
        set(_tools_dir tools/${CHORUSKIT_INSTALL_NAME})
    else()
        set(_tools_dir ${CMAKE_INSTALL_BINDIR})
    endif()

    install(TARGETS ${_target}
        EXPORT ${CHORUSKIT_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${_tools_dir}" OPTIONAL
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
    )
endfunction()

add_subdirectory(aec)