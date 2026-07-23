# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

include_guard(GLOBAL)

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
mark_as_advanced(ENABLE_COVERAGE)

block(SCOPE_FOR VARIABLES)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(cflags "-g;-Og;--coverage;-fkeep-inline-functions;-fkeep-static-functions")
        set(ldflags "--coverage")

        set(CACHE{COVERAGE_CFLAGS} TYPE STRING FORCE VALUE "${cflags}")
        set(CACHE{COVERAGE_CXXFLAGS} TYPE STRING FORCE VALUE "${cflags}")
        set(CACHE{COVERAGE_LDFLAGS} TYPE STRING FORCE VALUE "${ldflags}")

        add_custom_target(
            coverage_report
            COMMAND ${CMAKE_CURRENT_LIST_DIR}/collect_gcov_data.sh "html" "${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            VERBATIM
        )

        add_custom_target(
            coverage_data
            COMMAND ${CMAKE_CURRENT_LIST_DIR}/collect_gcov_data.sh "xml" "${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            VERBATIM
        )
    elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(cflags "-g;-O0;-fprofile-instr-generate;-fcoverage-mapping")
        set(ldflags "-fprofile-instr-generate;-Wl,--build-id")

        set(CACHE{COVERAGE_CFLAGS} TYPE STRING FORCE VALUE "${cflags}")
        set(CACHE{COVERAGE_CXXFLAGS} TYPE STRING FORCE VALUE "${cflags}")
        set(CACHE{COVERAGE_LDFLAGS} TYPE STRING FORCE VALUE "${ldflags}")

        add_custom_target(
            coverage_report
            COMMAND ${CMAKE_CURRENT_LIST_DIR}/collect_llvm_data.sh "html" "${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            VERBATIM
        )

        add_custom_target(
            coverage_data
            COMMAND ${CMAKE_CURRENT_LIST_DIR}/collect_llvm_data.sh "lcov" "${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            VERBATIM
        )
    else()
        message(WARNING "Coverage not supported for the current compiler")
    endif()

    mark_as_advanced(FORCE COVERAGE_CFLAGS COVERAGE_CXXFLAGS COVERAGE_LDFLAGS)
endblock()

function(target_enable_coverage target)
    target_compile_options(${target} PRIVATE "$<$<AND:$<BOOL:${ENABLE_COVERAGE}>,$<COMPILE_LANGUAGE:C>>:${COVERAGE_CFLAGS}>")
    target_compile_options(${target} PRIVATE "$<$<AND:$<BOOL:${ENABLE_COVERAGE}>,$<COMPILE_LANGUAGE:CXX>>:${COVERAGE_CXXFLAGS}>")
    target_link_options(${target} PUBLIC "$<$<BOOL:${ENABLE_COVERAGE}>:${COVERAGE_LDFLAGS}>")
endfunction()
