project(ExtensionSystem VERSION ${QT_CREATOR_VERSION} LANGUAGES CXX)

file(GLOB _src *.h *.cpp *.ui *.qrc newSettings/*.h newSettings/*.cpp)

configure_qtc_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE EXTENSIONSYSTEM_LIBRARY)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_DEFAULT_MAJOR_VERSION}::Widgets
    Aggregation
    Utils
)

set_source_files_properties(pluginmanager.cpp PROPERTIES SKIP_AUTOMOC ON)

choruskit_install_headers(. EXCLUDE_DIRS test newSettings)