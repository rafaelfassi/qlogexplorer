project(qlogexplorer_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/cmake/gtest.cmake)

add_executable(${PROJECT_NAME}
    qlogexplorer_test.qrc
    main.cpp
    TestUtils.h
    TestUtils.cpp

    # Tests
    Test_JsonLogModel.cpp
    Test_RegexMatcher.cpp
    Test_SubStringMatcher.cpp
    Test_TextLogModel.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        qlogexplorer_core
        ${GTEST_TARGETS}
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME} DISCOVERY_MODE PRE_TEST)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common/pch.h)
