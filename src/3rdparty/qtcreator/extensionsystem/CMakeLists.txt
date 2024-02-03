project(ExtensionSystem VERSION ${QT_CREATOR_VERSION} LANGUAGES CXX)

file(GLOB _src *.h *.cpp *.ui *.qrc newSettings/*.h newSettings/*.cpp)

configure_qtc_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS Aggregation Utils
    QT_LINKS Core Widgets
    SKIP_AUTOMOC "pluginmanager.cpp"
)