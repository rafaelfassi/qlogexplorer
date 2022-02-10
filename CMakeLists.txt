cmake_minimum_required(VERSION 3.1)

project(qlogexplorer LANGUAGES CXX)
set(VERSION 0.0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-pthread")

add_definitions(-DAPP_BASE_SRC_DIR="${CMAKE_SOURCE_DIR}")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(TBB)

add_definitions(-fPIC)

set(MAIN_SOURCES
    qlogexplorer.qrc
    src/main.cpp
    src/Types.h
    src/Types.cpp
    src/Conf.h
    src/Conf.cpp
    src/Utils.h
    src/Utils.cpp
    src/Settings.h
    src/Settings.cpp
)

set(MODEL_HEADERS
    src/model/AbstractModel.h
    src/model/BaseLogModel.h
    src/model/TextLogModel.h
    src/model/JsonLogModel.h
    src/model/ProxyModel.h
)

set(MODEL_SOURCES
    src/model/BaseLogModel.cpp
    src/model/TextLogModel.cpp
    src/model/JsonLogModel.cpp
    src/model/ProxyModel.cpp
)

set(MATCH_HEADERS
    src/match/BaseMatcher.h
    src/match/RegexMatcher.h
    src/match/SubStringMatcher.h
    src/match/RangeMatcher.h
    src/match/Matcher.h
)

set(MATCH_SOURCES
    src/match/RegexMatcher.cpp
    src/match/SubStringMatcher.cpp
    src/match/RangeMatcher.cpp
    src/match/Matcher.cpp
)

set(GUI_HEADERS
    src/gui/GuiTypes.h
    src/gui/Style.h
    src/gui/MainWindow.h
    src/gui/LongScrollBar.h
    src/gui/LogViewWidget.h
    src/gui/SearchParamWidget.h
    src/gui/LogSearchWidget.h
    src/gui/LogTabWidget.h
    src/gui/HeaderView.h
    src/gui/Highlighter.h
)

set(GUI_SOURCES
    src/gui/Style.cpp
    src/gui/MainWindow.cpp
    src/gui/LongScrollBar.cpp
    src/gui/LogViewWidget.cpp
    src/gui/SearchParamWidget.cpp
    src/gui/LogSearchWidget.cpp
    src/gui/LogTabWidget.cpp
    src/gui/HeaderView.cpp
    src/gui/Highlighter.cpp
)

add_executable(${PROJECT_NAME}
    ${MAIN_SOURCES}
    ${MODEL_HEADERS}
    ${MODEL_SOURCES}
    ${MATCH_HEADERS}
    ${MATCH_SOURCES}
    ${GUI_HEADERS}
    ${GUI_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    src
    src/model
    src/match
    src/gui
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(TBB_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE -ltbb)
    add_definitions(-DHAS_TBB_ENABLED)
else()
    MESSAGE(STATUS "TBB not found. Configure TBB to improve performance.")
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)
