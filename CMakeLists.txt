# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

project(sqlixx VERSION 1.0.0 LANGUAGES C CXX)

option(SQLIXX_BUILD_TESTING "Build unit tests for SQLixx" ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(ProjectDependencies)
include(EnableClangTidy)
include(EnableCoverage)
include(EnableSanitizers)

add_library(sqlixx STATIC)
add_library(SQLixx::SQLixx ALIAS sqlixx)

target_compile_features(sqlixx PUBLIC cxx_std_23)

set_target_properties(sqlixx PROPERTIES CXX_MODULE_STD ON)

target_sources(
    sqlixx
    PUBLIC
        FILE_SET CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/src/module.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/error.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/error.sqlite.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/handles.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/connection.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/connection.open.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/statement.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/statement.prepare.cppm"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/binders.cppm"
)

target_link_libraries(sqlixx PUBLIC SQLite3::SQLite3)

target_enable_clang_tidy(sqlixx)
target_enable_coverage(sqlixx)
target_enable_sanitizers(sqlixx)

if(SQLIXX_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# -- Install & Export ----------------------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ConfigPackageLocation "${CMAKE_INSTALL_LIBDIR}/cmake/SQLixx")

install(TARGETS sqlixx EXPORT SQLixxTargets FILE_SET CXX_MODULES DESTINATION "${ConfigPackageLocation}/src")

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/SQLixxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SQLixxConfig.cmake"
    INSTALL_DESTINATION "${ConfigPackageLocation}"
    NO_SET_AND_CHECK_MACRO
)

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/SQLixxConfigVersion.cmake" COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SQLixxConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/SQLixxConfigVersion.cmake" DESTINATION "${ConfigPackageLocation}")

install(EXPORT SQLixxTargets DESTINATION "${ConfigPackageLocation}" NAMESPACE SQLixx:: CXX_MODULES_DIRECTORY .)

# -- Packaging -----------------------------------------------------------------

if(PROJECT_IS_TOP_LEVEL)
    include(InstallRequiredSystemLibraries)
    include(CPack)
endif()
