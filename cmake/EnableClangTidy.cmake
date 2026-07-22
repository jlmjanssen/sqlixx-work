# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

include_guard(GLOBAL)

option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy during compilation" OFF)
mark_as_advanced(ENABLE_CLANG_TIDY)

function(target_enable_clang_tidy target)
    set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "$<$<BOOL:${ENABLE_CLANG_TIDY}>:clang-tidy>")
    target_compile_options(${target} PRIVATE "$<$<BOOL:${ENABLE_CLANG_TIDY}>:-Wall;-Wextra;-Wpedantic>")
endfunction()
