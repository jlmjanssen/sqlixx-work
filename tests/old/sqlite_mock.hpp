// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#pragma once

#include <gmock/gmock.h>
#include <sqlite3.h>

class SQLiteMock {
public:
    MOCK_METHOD(const char*, sqlite3_errstr, (int));
    MOCK_METHOD(const char*, sqlite3_errmsg, (::sqlite3*));

    MOCK_METHOD(int, sqlite3_open_v2, (const char*, sqlite3**, int, const char*));
    MOCK_METHOD(int, sqlite3_close_v2, (sqlite3*));
    MOCK_METHOD(int, sqlite3_prepare_v3, (sqlite3*, const char*, int, unsigned int, sqlite3_stmt**, const char**));
    MOCK_METHOD(int, sqlite3_finalize, (sqlite3_stmt*));

    MOCK_METHOD(int, sqlite3_bind_int, (sqlite3_stmt*, int, int));
    MOCK_METHOD(int, sqlite3_bind_int64, (sqlite3_stmt*, int, sqlite3_int64));
    MOCK_METHOD(int, sqlite3_bind_double, (sqlite3_stmt*, int, double));
    MOCK_METHOD(int, sqlite3_bind_text, (sqlite3_stmt*, int, const char*, int, void (*)(void*)));
    MOCK_METHOD(int, sqlite3_bind_blob64, (sqlite3_stmt*, int, const void*, sqlite3_uint64, void (*)(void*)));
    MOCK_METHOD(int, sqlite3_bind_zeroblob64, (sqlite3_stmt*, int, sqlite3_uint64));
    MOCK_METHOD(int, sqlite3_bind_null, (sqlite3_stmt*, int));
    MOCK_METHOD(int, sqlite3_bind_value, (sqlite3_stmt*, int, const sqlite3_value*));
    MOCK_METHOD(int, sqlite3_bind_parameter_index, (sqlite3_stmt*, const char*));

    MOCK_METHOD(int, sqlite3_data_count, (sqlite3_stmt*));
    MOCK_METHOD(int, sqlite3_column_int, (sqlite3_stmt*, int));
    MOCK_METHOD(sqlite3_int64, sqlite3_column_int64, (sqlite3_stmt*, int));
    MOCK_METHOD(double, sqlite3_column_double, (sqlite3_stmt*, int));
    MOCK_METHOD(const unsigned char*, sqlite3_column_text, (sqlite3_stmt*, int));
    MOCK_METHOD(const void*, sqlite3_column_blob, (sqlite3_stmt*, int));
    MOCK_METHOD(int, sqlite3_column_bytes, (sqlite3_stmt*, int));
    MOCK_METHOD(sqlite3_value*, sqlite3_column_value, (sqlite3_stmt*, int));
};

inline SQLiteMock* g_sqlite_mock{};
