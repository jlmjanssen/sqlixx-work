// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#include "sqlite_mock.hpp"

extern "C" {
const char* __real_sqlite3_errstr(int rc);
const char* __real_sqlite3_errmsg(sqlite3* db);
int __real_sqlite3_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs);
int __real_sqlite3_close_v2(sqlite3* db);
int __real_sqlite3_prepare_v3(
    sqlite3* db, const char* zSql, int nByte, unsigned int prepFlags, sqlite3_stmt** ppStmt, const char** pzTail);
int __real_sqlite3_finalize(sqlite3_stmt* pStmt);

int __real_sqlite3_bind_int64(sqlite3_stmt* pStmt, int i, sqlite3_int64 iValue);
int __real_sqlite3_bind_double(sqlite3_stmt* pStmt, int i, double rValue);
int __real_sqlite3_bind_text(sqlite3_stmt* pStmt, int i, const char* zData, int nData, void (*xDel)(void*));
int __real_sqlite3_bind_blob64(
    sqlite3_stmt* pStmt, int i, const void* zData, sqlite3_uint64 nData, void (*xDel)(void*));
int __real_sqlite3_bind_zeroblob64(sqlite3_stmt* pStmt, int i, sqlite3_uint64 nData);
int __real_sqlite3_bind_null(sqlite3_stmt* pStmt, int i);
int __real_sqlite3_bind_value(sqlite3_stmt* pStmt, int i, const sqlite3_value* pValue);
int __real_sqlite3_bind_parameter_index(sqlite3_stmt* pStmt, const char* zName);

const char* __wrap_sqlite3_errstr(int rc) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_errstr(rc);
    return __real_sqlite3_errstr(rc);
}

const char* __wrap_sqlite3_errmsg(sqlite3* db) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_errmsg(db);
    return __real_sqlite3_errmsg(db);
}

int __wrap_sqlite3_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_open_v2(filename, ppDb, flags, zVfs);
    return __real_sqlite3_open_v2(filename, ppDb, flags, zVfs);
}

int __wrap_sqlite3_close_v2(sqlite3* db) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_close_v2(db);
    return __real_sqlite3_close_v2(db);
}

int __wrap_sqlite3_prepare_v3(
    sqlite3* db, const char* zSql, int nByte, unsigned int prepFlags, sqlite3_stmt** ppStmt, const char** pzTail) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_prepare_v3(db, zSql, nByte, prepFlags, ppStmt, pzTail);
    return __real_sqlite3_prepare_v3(db, zSql, nByte, prepFlags, ppStmt, pzTail);
}

int __wrap_sqlite3_finalize(sqlite3_stmt* pStmt) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_finalize(pStmt);
    return __real_sqlite3_finalize(pStmt);
}

int __wrap_sqlite3_bind_int64(sqlite3_stmt* pStmt, int i, sqlite3_int64 iValue) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_int64(pStmt, i, iValue);
    return __real_sqlite3_bind_int64(pStmt, i, iValue);
}

int __wrap_sqlite3_bind_double(sqlite3_stmt* pStmt, int i, double rValue) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_double(pStmt, i, rValue);
    return __real_sqlite3_bind_double(pStmt, i, rValue);
}

int __wrap_sqlite3_bind_text(sqlite3_stmt* pStmt, int i, const char* zData, int nData, void (*xDel)(void*)) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_text(pStmt, i, zData, nData, xDel);
    return __real_sqlite3_bind_text(pStmt, i, zData, nData, xDel);
}

int __wrap_sqlite3_bind_blob64(
    sqlite3_stmt* pStmt, int i, const void* zData, sqlite3_uint64 nData, void (*xDel)(void*)) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_blob64(pStmt, i, zData, nData, xDel);
    return __real_sqlite3_bind_blob64(pStmt, i, zData, nData, xDel);
}

int __wrap_sqlite3_bind_zeroblob64(sqlite3_stmt* pStmt, int i, sqlite3_uint64 nData) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_zeroblob64(pStmt, i, nData);
    return __real_sqlite3_bind_zeroblob64(pStmt, i, nData);
}

int __wrap_sqlite3_bind_null(sqlite3_stmt* pStmt, int i) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_null(pStmt, i);
    return __real_sqlite3_bind_null(pStmt, i);
}

int __wrap_sqlite3_bind_value(sqlite3_stmt* pStmt, int i, const sqlite3_value* pValue) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_value(pStmt, i, pValue);
    return __real_sqlite3_bind_value(pStmt, i, pValue);
}

int __wrap_sqlite3_bind_parameter_index(sqlite3_stmt* pStmt, const char* zName) {
    if (g_sqlite_mock)
        return g_sqlite_mock->sqlite3_bind_parameter_index(pStmt, zName);
    return __real_sqlite3_bind_parameter_index(pStmt, zName);
}
}
