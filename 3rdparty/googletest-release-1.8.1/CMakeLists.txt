cmake_minimum_required(VERSION 2.8.8)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:Debug>:/MDd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MD> #--|
        $<$<CONFIG:RelWithDebInfo>:/MD> #--|
    )
endif()

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 OLD)
endif (POLICY CMP0048)

project(googletest-distribution)
set(GOOGLETEST_VERSION 1.9.0)

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON)

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
else()
  add_subdirectory( googletest )
endif()
