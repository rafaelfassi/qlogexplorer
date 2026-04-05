if(USE_INSTALLED_RE2)
    # Tries to find already system-installed RE2
    if(WIN32)
        # vcpkg provides re2::re2 (CONFIG mode)
        # When using vcpkg the following parameters need to be provided:
        # -DCMAKE_TOOLCHAIN_FILE="<PATH_TO_VCPKG_INSTALLATION_ROOT>/scripts/buildsystems/vcpkg.cmake"
        # -DVCPKG_TARGET_TRIPLET=<INSTALLED_RE2_PACKAGE>
        # Example:
        #   Install/configure static re2 for mingw 64 bits:
        #       vcpkg install re2:x64-mingw-static
        #   Run cmake with the parameters:
        #       -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        #       -DVCPKG_TARGET_TRIPLET=x64-mingw-static
        #   For MSVC the the package would be: x64-windows-static-md
        find_package(re2 CONFIG REQUIRED)
        set(RE2_TARGETS re2::re2)
    else()
        # For Linux it uses pkg-config to fins re2
        # You need to have the package "libre2-dev" installed, or similar package according to the linux distribution.
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(RE2 REQUIRED IMPORTED_TARGET re2)
        set(RE2_TARGETS PkgConfig::RE2)
    endif()
else()
    # Fetch and configure RE2
    include(FetchContent)

    # 1. Abseil (required by RE2)
    FetchContent_Declare (
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG 20250814.2
        GIT_SHALLOW TRUE
    )

    # 2. RE2
    FetchContent_Declare ( 
        re2
        GIT_REPOSITORY https://github.com/google/re2.git
        GIT_TAG 2025-11-05
        GIT_SHALLOW TRUE
    )

    set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
    set(RE2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(RE2_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(abseil re2)
    set(RE2_TARGETS re2::re2)
endif()
