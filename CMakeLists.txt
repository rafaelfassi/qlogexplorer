cmake_minimum_required(VERSION 3.10)

project(qlogexplorer VERSION 1.1.3 LANGUAGES CXX)
set(APP_NAME "QLogExplorer")
set(PROJ_URL "https://github.com/rafaelfassi/qlogexplorer")
set(APP_URL "https://rafaelfassi.github.io/qlogexplorer/")
set(WIKI_URL "https://github.com/rafaelfassi/qlogexplorer/wiki")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

option(USE_INSTALLED_RE2 "Use system-installed RE2 instead of downloading via FetchContent" OFF)
option(DISABLE_TESTS "Disable tests to make gtest not required" OFF)

# RE2 must be included before Qt and before AUTOMOC is set
include(3rdparty/re2/re2.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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

include(InstallRequiredSystemLibraries)

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(app)

if(NOT DISABLE_TESTS)
    add_subdirectory(tests)
endif()
