project(qlogexplorer_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    # vcpkg provides re2::re2 (CONFIG mode)
    find_package(re2 CONFIG REQUIRED)
    set(RE2_TARGET re2::re2)
else()
    # Linux uses pkg-config (libre2-dev)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RE2 REQUIRED IMPORTED_TARGET re2)
    set(RE2_TARGET PkgConfig::RE2)
endif()

add_library(${PROJECT_NAME} STATIC
    # Base
    # headers
    include/Types.h
    include/FileConf.h
    include/Utils.h
    include/InFileStream.h
    include/Notifier.h
    # src
    src/Types.cpp
    src/FileConf.cpp
    src/Utils.cpp
    src/InFileStream.cpp

    # Regex
    # headers
    include/regex/Regex.h
    include/regex/QtRegex.h
    include/regex/Re2Regex.h
    include/regex/RegexBuilder.h
    # src
    src/regex/QtRegex.cpp
    src/regex/Re2Regex.cpp
    src/regex/RegexBuilder.cpp

    # Models
    # headers
    include/model/AbstractModel.h
    include/model/BaseLogModel.h
    include/model/TextLogModel.h
    include/model/JsonLogModel.h
    include/model/ProxyModel.h
    include/model/SearchParamModel.h
    # src
    src/model/BaseLogModel.cpp
    src/model/TextLogModel.cpp
    src/model/JsonLogModel.cpp
    src/model/ProxyModel.cpp
    src/model/SearchParamModel.cpp

    # Matchers
    # headers
    include/match/BaseMatcher.h
    include/match/RegexMatcher.h
    include/match/SubStringMatcher.h
    include/match/RangeMatcher.h
    include/match/Matcher.h
    # src
    src/match/RegexMatcher.cpp
    src/match/SubStringMatcher.cpp
    src/match/RangeMatcher.cpp
    src/match/Matcher.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
    PUBLIC
        include
        ${CMAKE_SOURCE_DIR}/3rdparty
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${RE2_TARGET}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
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

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common/pch.h)
