# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
# SPDX-License-Identifier: CC0-1.0

include_guard(GLOBAL)

include(FetchContent)

FetchContent_Declare(SQLite3 GIT_REPOSITORY https://github.com/algoritnl/sqlite-cmake.git GIT_TAG main)

FetchContent_MakeAvailable(SQLite3)
