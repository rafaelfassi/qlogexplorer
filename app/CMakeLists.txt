
set(MAIN_SOURCES
    ${CMAKE_SOURCE_DIR}/qlogexplorer.qrc
    src/main.cpp
)

if(WIN32)
    list(APPEND MAIN_SOURCES ${CMAKE_SOURCE_DIR}/packaging/win/qlogexplorer.rc)
endif(WIN32)

# The main application uses the root project name
add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/qtsingleapplication qtsingleapplication_build)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/qtsingleapplication/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        qlogexplorer_core
        qlogexplorer_gui
        qtsingleapplication
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif(WIN32)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common/pch.h)
