# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

include_guard(GLOBAL)

include(FetchContent)

set(CACHE{INSTALL_GTEST} TYPE BOOL VALUE OFF)
set(CACHE{BUILD_GMOCK} TYPE BOOL FORCE VALUE ON)
mark_as_advanced(FORCE BUILD_GMOCK)

FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.17.0)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
