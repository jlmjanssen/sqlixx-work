// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;

#include <sqlite3.h>

export module sqlixx:readers;

import std;
import :error.sqlite;
import :error;
import :handles;
import :statement;

namespace sqlixx {

// ============================================================================
// Column Reader Infrastructure (Consistent Specialization Idiom)
// ============================================================================

template <typename T>
struct column_reader;

template <std::integral T>
    requires(sizeof(T) <= sizeof(int))
struct column_reader<T> {
    auto operator()(::sqlite3_stmt* stmt, int col) const noexcept -> T {
        return static_cast<T>(::sqlite3_column_int(stmt, col));
    }
};

template <std::integral T>
    requires(sizeof(T) == sizeof(std::int64_t))
struct column_reader<T> {
    auto operator()(::sqlite3_stmt* stmt, int col) const noexcept -> T {
        return static_cast<T>(::sqlite3_column_int64(stmt, col));
    }
};

template <std::floating_point T>
struct column_reader<T> {
    auto operator()(::sqlite3_stmt* stmt, int col) const noexcept -> T {
        return static_cast<T>(::sqlite3_column_double(stmt, col));
    }
};

template <>
struct column_reader<std::string_view> {
    auto operator()(::sqlite3_stmt* stmt, int col) const noexcept -> std::string_view {
        auto* text = reinterpret_cast<const char*>(::sqlite3_column_text(stmt, col));
        int bytes = ::sqlite3_column_bytes(stmt, col);
        if (text == nullptr)
            return {};
        return {text, static_cast<std::size_t>(bytes)};
    }
};

// ============================================================================
// Exported Column & Row Extraction API (Now with std::expected!)
// ============================================================================

export template <typename T>
[[nodiscard]] auto read_column(statement_handle stmt, int col) noexcept -> std::expected<T, std::error_code> {
    if (col < 0 || col >= ::sqlite3_column_count(stmt.get())) {
        return std::unexpected(make_error_code(errc::invalid_argument));
    }

    if (::sqlite3_column_type(stmt.get(), col) == SQLITE_NULL) {
        return std::unexpected(make_error_code(errc::invalid_argument));
    }

    return column_reader<T>{}(stmt.get(), col);
}

export template <typename... Ts>
[[nodiscard]] auto read_row(statement_handle stmt) noexcept -> std::expected<std::tuple<Ts...>, std::error_code> {
    constexpr std::size_t expected_cols = sizeof...(Ts);
    if (static_cast<int>(expected_cols) > ::sqlite3_column_count(stmt.get())) {
        return std::unexpected(make_error_code(errc::invalid_argument));
    }

    int col = 0;
    std::error_code ec{};

    auto read_each = [&]<typename T>() -> T {
        if (ec)
            return T{};
        auto res = read_column<T>(stmt, col++);
        if (!res) {
            ec = res.error();
            return T{};
        }
        return *res;
    };

    std::tuple<Ts...> row_tuple{read_each.template operator()<Ts>()...};

    if (ec) {
        return std::unexpected(ec);
    }
    return row_tuple;
}

} // namespace sqlixx
