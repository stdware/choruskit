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
)

if(CHORUSKIT_INSTALL)
    choruskit_install_tool(${PROJECT_NAME})
endif()