project(qlogexplorer_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(tests_common.cmake)

add_executable(tests_app
    Test_SubStringMatcher.cpp
)

target_link_libraries(tests_app
    PRIVATE
        core_lib
        ${TEST_COMMON_LIBS}
)

include(GoogleTest)
gtest_discover_tests(tests_app DISCOVERY_MODE PRE_TEST)

target_precompile_headers(tests_app PRIVATE ${CMAKE_SOURCE_DIR}/common/pch.h)
