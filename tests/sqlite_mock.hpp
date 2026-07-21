// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#pragma once

#include <gmock/gmock.h>
#include <sqlite3.h>

class SQLiteMock {
public:
    MOCK_METHOD(const char*, sqlite3_errstr, (int), (noexcept));
    MOCK_METHOD(const char*, sqlite3_errmsg, (::sqlite3*), (noexcept));

    MOCK_METHOD(int, sqlite3_open_v2, (const char*, sqlite3**, int, const char*), (noexcept));
    MOCK_METHOD(int, sqlite3_close_v2, (sqlite3*), (noexcept));
    MOCK_METHOD(int,
                sqlite3_prepare_v3,
                (sqlite3*, const char*, int, unsigned int, sqlite3_stmt**, const char**),
                (noexcept));
    MOCK_METHOD(int, sqlite3_finalize, (sqlite3_stmt*), (noexcept));

    MOCK_METHOD(int, sqlite3_bind_int64, (sqlite3_stmt*, int, sqlite3_int64), (noexcept));
    MOCK_METHOD(int, sqlite3_bind_double, (sqlite3_stmt*, int, double), (noexcept));
    MOCK_METHOD(int, sqlite3_bind_text, (sqlite3_stmt*, int, const char*, int, void (*)(void*)), (noexcept));
    MOCK_METHOD(int,
                sqlite3_bind_blob64,
                (sqlite3_stmt*, int, const void*, sqlite3_uint64, void (*)(void*)),
                (noexcept));
    MOCK_METHOD(int, sqlite3_bind_zeroblob64, (sqlite3_stmt*, int, sqlite3_uint64), (noexcept));
    MOCK_METHOD(int, sqlite3_bind_null, (sqlite3_stmt*, int), (noexcept));
    MOCK_METHOD(int, sqlite3_bind_value, (sqlite3_stmt*, int, const sqlite3_value*), (noexcept));
    MOCK_METHOD(int, sqlite3_bind_parameter_index, (sqlite3_stmt*, const char*), (noexcept));
};

inline SQLiteMock* g_sqlite_mock{};
