cmake_minimum_required(VERSION 3.10)

project(qlogexplorer VERSION 1.1.1 LANGUAGES CXX)
set(APP_NAME "QLogExplorer")
set(PROJ_URL "https://github.com/rafaelfassi/qlogexplorer")
set(APP_URL "https://rafaelfassi.github.io/qlogexplorer/")
set(WIKI_URL "https://github.com/rafaelfassi/qlogexplorer/wiki")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_definitions(-DAPP_VERSION="${CMAKE_PROJECT_VERSION}")
add_definitions(-DAPP_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_definitions(-DAPP_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_definitions(-DAPP_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})
add_definitions(-DAPP_NAME="${APP_NAME}")
add_definitions(-DPROJ_URL="${PROJ_URL}")
add_definitions(-DAPP_URL="${APP_URL}")
add_definitions(-DWIKI_URL="${WIKI_URL}")
add_definitions(-DAPP_BASE_SRC_DIR="${CMAKE_SOURCE_DIR}")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)
find_package(Threads REQUIRED)

set(MAIN_SOURCES
    qlogexplorer.qrc
    src/main.cpp
    src/Types.h
    src/Types.cpp
    src/FileConf.h
    src/FileConf.cpp
    src/Utils.h
    src/Utils.cpp
    src/Settings.h
    src/Settings.cpp
    src/InFileStream.h
    src/InFileStream.cpp
)

if(WIN32)
    list(APPEND MAIN_SOURCES packaging/win/qlogexplorer.rc)
endif(WIN32)

set(MODEL_HEADERS
    src/model/AbstractModel.h
    src/model/BaseLogModel.h
    src/model/TextLogModel.h
    src/model/JsonLogModel.h
    src/model/ProxyModel.h
    src/model/SearchParamModel.h
)

set(MODEL_SOURCES
    src/model/BaseLogModel.cpp
    src/model/TextLogModel.cpp
    src/model/JsonLogModel.cpp
    src/model/ProxyModel.cpp
    src/model/SearchParamModel.cpp
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
    src/gui/Style.h
    src/gui/MainWindow.h
    src/gui/LongScrollBar.h
    src/gui/LogViewWidget.h
    src/gui/SearchParamControl.h
    src/gui/SearchParamWidget.h
    src/gui/LogSearchWidget.h
    src/gui/LogTabWidget.h
    src/gui/HeaderView.h
    src/gui/Highlighter.h
    src/gui/ProgressLabel.h
    src/gui/TemplatesConfigDlg.h
    src/gui/SettingsDlg.h
)

set(GUI_SOURCES
    src/gui/Style.cpp
    src/gui/MainWindow.cpp
    src/gui/LongScrollBar.cpp
    src/gui/LogViewWidget.cpp
    src/gui/SearchParamControl.cpp
    src/gui/SearchParamWidget.cpp
    src/gui/LogSearchWidget.cpp
    src/gui/LogTabWidget.cpp
    src/gui/HeaderView.cpp
    src/gui/Highlighter.cpp
    src/gui/ProgressLabel.cpp
    src/gui/TemplatesConfigDlg.cpp
    src/gui/SettingsDlg.cpp
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

add_subdirectory(3rdparty/qtsingleapplication)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    QtSolutions::SingleApplication
    Threads::Threads
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR MINGW)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        message("Compiler does not support parallel algorithms.")
        add_definitions(-DNO_STD_PARALLEL_ALGORITHMS)
    elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        # GCC 9 supports parallel algorithms, but it requires libtbb.
        # FIXME: It should be applied for Clang as well, looking at libstdc++ version.
        target_link_libraries(${PROJECT_NAME} PRIVATE -ltbb)
    endif()
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif(WIN32)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
