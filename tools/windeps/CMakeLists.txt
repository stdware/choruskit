project(ckwindeps VERSION 0.0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

add_executable(ChorusKit::windeps ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME windeps)

target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)

if(WIN32)
    string(TIMESTAMP _year "%Y")
    ck_attach_win_rc_file(${PROJECT_NAME}
        COPYRIGHT "Copyright ${CK_DEV_START_YEAR}-${_year} SineStriker"
        DESCRIPTION "ChorusKit Windows Deploy Tool"
    )
endif()

if(CHORUSKIT_INSTALL)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${CHORUSKIT_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
    )
endif()
