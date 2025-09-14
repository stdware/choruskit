project(CkAppCore
    VERSION ${CHORUSKIT_VERSION}
    LANGUAGES CXX
)

qm_import(Qml)

file(GLOB_RECURSE _src *.h *.cpp *.qrc)

find_package(stdcorelib CONFIG REQUIRED)

choruskit_add_library(${PROJECT_NAME} SHARED AUTOGEN
    SOURCES ${_src}
    QT_LINKS
        Core Widgets Network Qml
    QT_INCLUDE_PRIVATE
        Core Gui Widgets
    LINKS_PRIVATE
        stdcorelib::stdcorelib
    INCLUDE_PRIVATE
        *
    FEATURES
        cxx_std_20
    SYNC_INCLUDE_PREFIX
        CoreApi
)

if(QT_KNOWN_POLICY_QTP0001)
    qt_policy(SET QTP0001 NEW)
endif()
if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

qt_add_qml_module(${PROJECT_NAME}
    URI ChorusKit.AppCore
    QML_FILES
    OUTPUT_DIRECTORY ${QMSETUP_BUILD_DIR}/qml/ChorusKit/AppCore
)

qm_install_qml_modules(${PROJECT_NAME})