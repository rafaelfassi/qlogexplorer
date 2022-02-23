cmake_minimum_required(VERSION 3.10)

project(qtsingleapplication LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC
    src/qtlocalpeer.cpp
    src/qtlocalpeer.h
    src/qtsingleapplication.cpp
    src/qtsinglecoreapplication.h
    src/qtsinglecoreapplication.cpp
    include/qtsingleapplication.h
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
    PUBLIC
        include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

add_library(QtSolutions::SingleApplication ALIAS ${PROJECT_NAME})
