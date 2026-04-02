cmake_policy(SET CMP0135 NEW)

# Do not install GTest
set(INSTALL_GTEST OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

#include(${DBM_ROOT_DIR}/common/common.cmake)

# set(DBM_TEST_COMMON_HEADERS
#     ${DBM_COMMON_DIR}/tests/TestsCommon.hpp
#     ${DBM_COMMON_DIR}/tests/TestUtils.hpp
# )

# set(DBM_TEST_COMMON_SRC
#     ${DBM_COMMON_DIR}/tests/TestUtils.cpp
# )

set(TEST_COMMON_LIBS
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)
