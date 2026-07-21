// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;

#include <sqlite3.h>

export module sqlixx:readers;

import std;
import :error;
import :statement;

namespace sqlixx {

template <typename T>
struct column_reader;

template <std::integral T>
struct column_reader<T> {
    auto operator()(::sqlite3_stmt* stmt, int column) const noexcept -> T {
        if constexpr (std::is_same_v<T, bool>) {
            return ::sqlite3_column_int(stmt, column) != 0;
        } else if constexpr (sizeof(T) <= sizeof(int)) {
            return static_cast<T>(::sqlite3_column_int(stmt, column));
        } else {
            return static_cast<T>(::sqlite3_column_int64(stmt, column));
        }
    }
};

template <std::floating_point T>
struct column_reader<T> {
    auto operator()(::sqlite3_stmt* stmt, int column) const noexcept -> T {
        return static_cast<T>(::sqlite3_column_double(stmt, column));
    }
};

template <>
struct column_reader<std::string_view> {
    auto operator()(::sqlite3_stmt* stmt, int column) const noexcept -> std::string_view {
        const auto* const void_text = static_cast<const void*>(::sqlite3_column_text(stmt, column));
        if (void_text == nullptr) [[unlikely]] {
            return {};
        }
        const auto* const text = static_cast<const char*>(void_text);
        const int bytes = ::sqlite3_column_bytes(stmt, column);
        return {text, static_cast<std::size_t>(bytes)};
    }
};

template <>
struct column_reader<std::string> {
    auto operator()(::sqlite3_stmt* stmt, int column) const noexcept -> std::string {
        std::string_view view = column_reader<std::string_view>{}(stmt, column);
        return std::string{view};
    }
};

template <>
struct column_reader<std::span<const std::byte>> {
    auto operator()(::sqlite3_stmt* stmt, int column) const noexcept -> std::span<const std::byte> {
        const auto* data = static_cast<const std::byte*>(::sqlite3_column_blob(stmt, column));
        if (data == nullptr) [[unlikely]] {
            return {};
        }
        const int bytes = ::sqlite3_column_bytes(stmt, column);
        return {data, static_cast<std::size_t>(bytes)};
    }
};

template <>
struct column_reader<std::vector<std::byte>> {
    auto operator()(::sqlite3_stmt* stmt, int column) const noexcept -> std::vector<std::byte> {
        std::span<const std::byte> span = column_reader<std::span<const std::byte>>{}(stmt, column);
        return std::vector<std::byte>{span.begin(), span.end()};
    }
};

template <typename T>
    requires std::is_same_v<std::remove_const_t<T>, ::sqlite3_value>
struct column_reader<T*> {
    auto operator()(::sqlite3_stmt* stmt, int column) const noexcept -> const ::sqlite3_value* {
        return ::sqlite3_column_value(stmt, column);
    }
};

export template <typename T>
[[nodiscard]] auto read_column_at(statement_handle stmt, int column) noexcept -> std::expected<T, std::error_code> {
    const int available_columns = ::sqlite3_data_count(stmt.get());
    if (available_columns == 0) [[unlikely]] {
        return std::unexpected(errc::no_active_row);
    }
    if (column < 0 || column >= available_columns) [[unlikely]] {
        return std::unexpected(errc::invalid_column_index);
    }
    return column_reader<T>{}(stmt.get(), column);
}

export template <typename T>
auto read_column(statement_handle stmt, int& column) noexcept -> std::expected<T, std::error_code> {
    return read_column_at<T>(stmt, column++);
}

export template <typename... Args>
auto read_columns_from(statement_handle stmt, int& column, Args&... args) noexcept
    -> std::expected<void, std::error_code> {
    const int available_columns = ::sqlite3_data_count(stmt.get());
    constexpr int num_args = static_cast<int>(sizeof...(Args));
    if (available_columns == 0) [[unlikely]] {
        return std::unexpected(errc::no_active_row);
    }
    if (column < 0 || column + num_args > available_columns) [[unlikely]] {
        return std::unexpected(errc::invalid_column_index);
    }

    ([stmt,
      &column,
      &args]() noexcept { args = column_reader<std::remove_cvref_t<decltype(args)>>{}(stmt.get(), column++); }(),
     ...);

    return {};
}

export template <typename... Args>
auto read_columns_at(statement_handle stmt, int column, Args&... args) noexcept
    -> std::expected<void, std::error_code> {
    int current_column = column;
    return read_columns_from(stmt, current_column, args...);
}

export template <typename... Args>
auto read_columns(statement_handle stmt, Args&... args) noexcept -> std::expected<void, std::error_code> {
    return read_columns_at(stmt, 0, args...);
}

export template <typename Tuple>
auto read_tuple_from(statement_handle stmt, int& column, Tuple& tuple) noexcept
    -> std::expected<void, std::error_code> {
    return std::apply([stmt, &column](auto&... args) { return read_columns_from(stmt, column, args...); }, tuple);
}

export template <typename... Ts>
[[nodiscard]] auto read_tuple_at(statement_handle stmt, int column) noexcept
    -> std::expected<std::tuple<Ts...>, std::error_code> {
    int current_column = column;
    std::tuple<Ts...> result;

    auto status = read_tuple_from(stmt, current_column, result);
    if (!status) [[unlikely]] {
        return std::unexpected(status.error());
    }

    return result;
}

export template <typename... Ts>
[[nodiscard]] auto read_tuple(statement_handle stmt) noexcept -> std::expected<std::tuple<Ts...>, std::error_code> {
    return read_tuple_at<Ts...>(stmt, 0);
}

} // namespace sqlixx
