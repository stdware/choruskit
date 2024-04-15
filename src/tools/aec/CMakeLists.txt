project(ckaec
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

add_executable(${PROJECT_NAME})

file(GLOB _src *.h *.cpp)
qm_configure_target(${PROJECT_NAME}
    SOURCES ${_src}
    QT_LINKS Core
    LINKS $<BUILD_INTERFACE:xmladaptor>
    DEFINES APP_VERSION="${PROJECT_VERSION}"
    FEATURES cxx_std_17
)

qm_add_win_rc(${PROJECT_NAME}
    NAME ${CHORUSKIT_INSTALL_NAME}
    DESCRIPTION "ChorusKit Action Extension Compiler"
    COPYRIGHT ${CHORUSKIT_PROJECT_COPYRIGHT}
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE
)

if(CHORUSKIT_INSTALL)
    choruskit_install_tool(${PROJECT_NAME})
endif()