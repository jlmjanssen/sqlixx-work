// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;

#include <sqlite3.h>

export module sqlixx:sqlite_error;
import std;

namespace sqlixx {

export enum class sqlite_errc : int {
    error = SQLITE_ERROR,
    perm = SQLITE_PERM,
    abort = SQLITE_ABORT,
    busy = SQLITE_BUSY,
    locked = SQLITE_LOCKED,
    nomem = SQLITE_NOMEM,
    readonly = SQLITE_READONLY,
    interrupt = SQLITE_INTERRUPT,
    ioerr = SQLITE_IOERR,
    corrupt = SQLITE_CORRUPT,
    notfound = SQLITE_NOTFOUND,
    full = SQLITE_FULL,
    cantopen = SQLITE_CANTOPEN,
    protocol = SQLITE_PROTOCOL,
    schema = SQLITE_SCHEMA,
    toobig = SQLITE_TOOBIG,
    constraint = SQLITE_CONSTRAINT,
    mismatch = SQLITE_MISMATCH,
    misuse = SQLITE_MISUSE,
    nolfs = SQLITE_NOLFS,
    auth = SQLITE_AUTH,
    range = SQLITE_RANGE,
    notadb = SQLITE_NOTADB
};

class sqlite_error_category final : public std::error_category {
public:
    [[nodiscard]] auto name() const noexcept -> const char* override { return "sqlite3"; }

    [[nodiscard]] auto message(int code) const -> std::string override { return ::sqlite3_errstr(code); }

    [[nodiscard]] auto default_error_condition(int code) const noexcept -> std::error_condition override {
        return {convert_to_primary_result_code(code), *this};
    }

private:
    static constexpr int primary_result_code_mask = 0xFF;

    static constexpr auto convert_to_primary_result_code(int code) noexcept -> int {
        return code & primary_result_code_mask;
    }
};

export [[nodiscard]] auto sqlite_category() noexcept -> const std::error_category& {
    static sqlite_error_category instance;
    return instance;
}

export [[nodiscard]] auto make_sqlite_error_code(int code) noexcept -> std::error_code {
    return {code, sqlite_category()};
}

export [[nodiscard]] auto make_error_condition(sqlite_errc cond) noexcept -> std::error_condition {
    return {std::to_underlying(cond), sqlite_category()};
}
} // namespace sqlixx

namespace std {
template <>
struct is_error_condition_enum<sqlixx::sqlite_errc> : std::true_type {};
} // namespace std
