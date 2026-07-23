# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

include_guard(GLOBAL)

option(ENABLE_SANITIZER_ADDRESS "Enable AddressSanitizer (ASan)" OFF)
option(ENABLE_SANITIZER_UNDEFINED "Enable UndefinedBehaviorSanitizer (UBSan)" OFF)
mark_as_advanced(ENABLE_SANITIZER_ADDRESS ENABLE_SANITIZER_UNDEFINED)

function(target_enable_sanitizers target)
    target_compile_options(
        ${target}
        PRIVATE
            "$<$<BOOL:${ENABLE_SANITIZER_ADDRESS}>:-fsanitize=address;-fno-omit-frame-pointer>"
            "$<$<BOOL:${ENABLE_SANITIZER_UNDEFINED}>:-fsanitize=undefined>"
    )
    target_link_options(
        ${target}
        PUBLIC "$<$<BOOL:${ENABLE_SANITIZER_ADDRESS}>:-fsanitize=address>" "$<$<BOOL:${ENABLE_SANITIZER_UNDEFINED}>:-fsanitize=undefined>"
    )
endfunction()
