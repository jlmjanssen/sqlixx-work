// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#include "sqlite_mock.hpp"

extern "C" {
const char* __real_sqlite3_errstr(int rc);
int __real_sqlite3_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs);
int __real_sqlite3_close_v2(sqlite3* db);
int __real_sqlite3_prepare_v3(
    sqlite3* db, const char* zSql, int nByte, unsigned int prepFlags, sqlite3_stmt** ppStmt, const char** pzTail);
int __real_sqlite3_finalize(sqlite3_stmt* pStmt);

const char* __wrap_sqlite3_errstr(int rc) {
    if (g_sqlite_mock) {
        return g_sqlite_mock->sqlite3_errstr(rc);
    }
    return __real_sqlite3_errstr(rc);
}

int __wrap_sqlite3_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs) {
    if (g_sqlite_mock) {
        return g_sqlite_mock->sqlite3_open_v2(filename, ppDb, flags, zVfs);
    }
    return __real_sqlite3_open_v2(filename, ppDb, flags, zVfs);
}

int __wrap_sqlite3_close_v2(sqlite3* db) {
    if (g_sqlite_mock) {
        return g_sqlite_mock->sqlite3_close_v2(db);
    }
    return __real_sqlite3_close_v2(db);
}

int __wrap_sqlite3_prepare_v3(
    sqlite3* db, const char* zSql, int nByte, unsigned int prepFlags, sqlite3_stmt** ppStmt, const char** pzTail) {
    if (g_sqlite_mock) {
        return g_sqlite_mock->sqlite3_prepare_v3(db, zSql, nByte, prepFlags, ppStmt, pzTail);
    }
    return __real_sqlite3_prepare_v3(db, zSql, nByte, prepFlags, ppStmt, pzTail);
}

int __wrap_sqlite3_finalize(sqlite3_stmt* pStmt) {
    if (g_sqlite_mock) {
        return g_sqlite_mock->sqlite3_finalize(pStmt);
    }
    return __real_sqlite3_finalize(pStmt);
}
}
