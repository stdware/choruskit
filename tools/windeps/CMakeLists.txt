project(ckwindeps VERSION 0.0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)

if(WIN32)
    ck_attach_win_rc_file(${PROJECT_NAME}
        VENDOR "Sine Striker"
        DESCRIPTION "ChorusKit Windows Deploy Tool"
    )
endif()

if(CHORUSKIT_INSTALL)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${CHORUSKIT_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
    )
endif()
