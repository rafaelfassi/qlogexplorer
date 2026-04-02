project(qlogexplorer_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(tests_common.cmake)

add_executable(${PROJECT_NAME}
    Test_SubStringMatcher.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        qlogexplorer_core
        ${TEST_COMMON_LIBS}
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME} DISCOVERY_MODE PRE_TEST)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common/pch.h)
