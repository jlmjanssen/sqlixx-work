// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#pragma once

#include <gmock/gmock.h>
#include <sqlite3.h>

class SQLiteMock {
public:
    MOCK_METHOD(const char*, sqlite3_errstr, (int), (noexcept));
    MOCK_METHOD(int, sqlite3_open_v2, (const char*, sqlite3**, int, const char*), (noexcept));
    MOCK_METHOD(int, sqlite3_close_v2, (sqlite3*), (noexcept));
    MOCK_METHOD(int,
                sqlite3_prepare_v3,
                (sqlite3*, const char*, int, unsigned int, sqlite3_stmt**, const char**),
                (noexcept));
    MOCK_METHOD(int, sqlite3_finalize, (sqlite3_stmt*), (noexcept));
};

inline SQLiteMock* g_sqlite_mock{};
