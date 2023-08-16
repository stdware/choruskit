project(Aggregation VERSION ${QT_CREATOR_VERSION} LANGUAGES CXX)

file(GLOB _src *.h *.cpp)

configure_qtc_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE AGGREGATION_LIBRARY)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_DEFAULT_MAJOR_VERSION}::Core
)

choruskit_install_headers(. EXCLUDE_DIRS examples)