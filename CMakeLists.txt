cmake_minimum_required(VERSION 3.1)

project(qlogviewer LANGUAGES CXX)
set(VERSION 0.0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-pthread")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

add_definitions(-fPIC)

set(PROJECT_HEADERS
    MainWindow.h
    AbstractModel.h
    BaseLogModel.h
    TextLogModel.h
    JsonLogModel.h
    LongScrollBar.h
    LogViewWidget.h
    SearchParamWidget.h
    LogSearchWidget.h
    LogTabWidget.h
    ProxyModel.h
    HeaderView.h
)

set(PROJECT_SOURCES
    qlogviewer.qrc
    main.cpp
    MainWindow.cpp
    BaseLogModel.cpp
    TextLogModel.cpp
    JsonLogModel.cpp
    LongScrollBar.cpp
    LogViewWidget.cpp
    SearchParamWidget.cpp
    LogSearchWidget.cpp
    LogTabWidget.cpp
    ProxyModel.cpp
    HeaderView.cpp
)

add_executable(qlogviewer
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_link_libraries(qlogviewer
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
)

