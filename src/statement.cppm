// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;
#include <sqlite3.h>

export module sqlixx:statement;
import std;
import :error;
import :sqlite_error;
import :handles;
import :connection;

namespace sqlixx {

export using statement_handle = shallow_handle<::sqlite3_stmt*>;
export using statement = owning_handle<::sqlite3_stmt*>;

template <>
struct handle_deleter<::sqlite3_stmt*> {
    auto operator()(::sqlite3_stmt* handle) const noexcept -> void { ::sqlite3_finalize(handle); }
};

export struct prepare_result {
    statement stmt;
    std::string_view tail;
};

namespace prep {
struct flag_t {};

template <unsigned int flags>
struct flags_t : flag_t {
    [[nodiscard]] constexpr auto get() const noexcept -> unsigned int { return flags; }
};

export struct dyn_flags : flag_t {
    explicit constexpr dyn_flags(unsigned int flags) noexcept : flags_(flags) {}
    [[nodiscard]] constexpr auto get() const noexcept -> unsigned int { return flags_; }

private:
    unsigned int flags_;
};

using persistent_t = flags_t<SQLITE_PREPARE_PERSISTENT>;
using normalize_t = flags_t<SQLITE_PREPARE_NORMALIZE>;
using no_vtab_t = flags_t<SQLITE_PREPARE_NO_VTAB>;
using dont_log_t = flags_t<SQLITE_PREPARE_DONT_LOG>;
using from_ddl_t = flags_t<SQLITE_PREPARE_FROM_DDL>;

export constexpr persistent_t persistent;
export constexpr normalize_t normalize;
export constexpr no_vtab_t no_vtab;
export constexpr dont_log_t dont_log;
export constexpr from_ddl_t from_ddl;

template <typename Flag>
concept is_flag = std::is_base_of_v<flag_t, std::decay_t<Flag>> && requires(std::decay_t<Flag> flags) {
    { flags.get() } noexcept -> std::convertible_to<unsigned int>;
};

template <typename... Flags>
concept all_flags = (is_flag<Flags> && ...);

template <typename... Flags>
    requires all_flags<Flags...>
constexpr auto make_flags(Flags... flags) noexcept -> unsigned int {
    return (0U | ... | flags.get());
}
} // namespace prep

[[nodiscard]] constexpr auto
prepare_statement_impl(::sqlite3* db_handle, const char* sql_ptr, int byte_count, unsigned int prep_flags) noexcept
    -> std::expected<prepare_result, std::error_code> {
    if (db_handle == nullptr) {
        return std::unexpected(errc::invalid_handle);
    }

    ::sqlite3_stmt* stmt_handle = nullptr;
    const char* tail_ptr = nullptr;

    int result = ::sqlite3_prepare_v3(db_handle, sql_ptr, byte_count, prep_flags, &stmt_handle, &tail_ptr);

    if (result != SQLITE_OK) {
        return std::unexpected(make_sqlite_error_code(result));
    }

    std::string_view tail{};

    if (byte_count < 0) {
        tail = std::string_view(tail_ptr);
    } else {
        std::ptrdiff_t consumed_bytes = tail_ptr - sql_ptr;
        if (consumed_bytes < byte_count) {
            tail = std::string_view(tail_ptr, static_cast<std::size_t>(byte_count - consumed_bytes));
        }
    }

    return prepare_result{.stmt = statement(stmt_handle), .tail = tail};
}

export template <prep::is_flag... Flags>
[[nodiscard]] constexpr auto prepare_statement(connection_handle conn, const char* sql, Flags... flags) noexcept
    -> std::expected<prepare_result, std::error_code> {
    return prepare_statement_impl(conn.c_handle(), sql, -1, prep::make_flags(flags...));
}

export template <prep::is_flag... Flags>
[[nodiscard]] constexpr auto prepare_statement(connection_handle conn, std::string_view sql, Flags... flags) noexcept
    -> std::expected<prepare_result, std::error_code> {
    return prepare_statement_impl(
        conn.c_handle(), sql.data(), static_cast<int>(sql.size()), prep::make_flags(flags...));
}

} // namespace sqlixx
