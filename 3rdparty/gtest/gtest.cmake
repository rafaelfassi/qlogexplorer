cmake_policy(SET CMP0135 NEW)

# Do not install GTest
set(INSTALL_GTEST OFF)

include(FetchContent)
FetchContent_Declare (
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
  GIT_SHALLOW TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(GTEST_TARGETS
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
    GTest::gmock_main
)
