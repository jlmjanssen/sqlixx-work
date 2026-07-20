# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

include(TestDependencies)
include(GoogleTest)

add_executable(sqlixx_test)
set_target_properties(sqlixx_test PROPERTIES CXX_MODULE_STD ON)
target_sources(
    sqlixx_test
    PRIVATE sqlite_mock.cpp error_test.cpp sqlite_error_test.cpp connection_test.cpp statement_test.cpp
)
target_link_libraries(sqlixx_test PRIVATE SQLixx::SQLixx GTest::gmock_main $<$<CXX_COMPILER_ID:GNU>:-lstdc++exp>)
target_link_options(
    sqlixx_test
    PRIVATE
        "LINKER:--wrap=sqlite3_errstr"
        "LINKER:--wrap=sqlite3_open_v2"
        "LINKER:--wrap=sqlite3_close_v2"
        "LINKER:--wrap=sqlite3_prepare_v3"
        "LINKER:--wrap=sqlite3_finalize"
)
target_enable_coverage(sqlixx_test)
gtest_add_tests(TARGET sqlixx_test)
