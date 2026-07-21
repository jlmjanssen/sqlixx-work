// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;

#include <sqlite3.h>

export module sqlixx:binders;

import std;
import :error.sqlite;
import :statement;

namespace sqlixx {

export enum class bind : std::uint8_t { copy, shallow };

// NOLINTBEGIN(cppcoreguidelines-*,performance-*)
inline const auto sqlite_static = reinterpret_cast<::sqlite3_destructor_type>(0);
inline const auto sqlite_transient = reinterpret_cast<::sqlite3_destructor_type>(-1);
// NOLINTEND(cppcoreguidelines-*,performance-*)

export struct zeroblob {
    std::size_t size{};
};

template <typename T>
struct parameter_binder;

template <std::integral T>
struct parameter_binder<T> {
    auto operator()(::sqlite3_stmt* stmt, int index, T value) const noexcept -> int {
        if constexpr (sizeof(T) <= sizeof(std::int64_t)) {
            // Beware: uint64_t values with the high bit set will overflow.
            return ::sqlite3_bind_int64(stmt, index, static_cast<std::int64_t>(value));
        } else {
            static_assert(sizeof(T) == 0, "Unsupported integral size");
        }
    }
};

template <std::floating_point T>
struct parameter_binder<T> {
    auto operator()(::sqlite3_stmt* stmt, int index, T value) const noexcept -> int {
        if constexpr (sizeof(T) <= sizeof(double)) {
            return ::sqlite3_bind_double(stmt, index, static_cast<double>(value));
        } else {
            static_assert(sizeof(T) == 0, "Unsupported floating point size");
        }
    }
};

template <typename T>
    requires std::convertible_to<std::decay_t<T>, std::string_view>
struct parameter_binder<T> {
    auto operator()(::sqlite3_stmt* stmt, int index, const T& value, bind strategy = bind::copy) const noexcept -> int {
        auto lifetime = (strategy == bind::copy) ? sqlite_transient : sqlite_static;
        if constexpr (std::is_pointer_v<T>) {
            return ::sqlite3_bind_text(stmt, index, static_cast<const char*>(value), -1, lifetime);
        } else if constexpr (std::is_array_v<T>) {
            constexpr std::size_t length = sizeof(T) - 1;
            return ::sqlite3_bind_text(
                stmt, index, static_cast<const char*>(value), static_cast<int>(length), lifetime);
        } else {
            std::string_view view = value;
            return ::sqlite3_bind_text(stmt, index, view.data(), static_cast<int>(view.size()), lifetime);
        }
    }
};

template <typename T, std::size_t Extent>
    requires std::is_trivially_copyable_v<T>
struct parameter_binder<std::span<T, Extent>> {
    auto
    operator()(::sqlite3_stmt* stmt, int index, std::span<T, Extent> value, bind strategy = bind::copy) const noexcept
        -> int {
        auto lifetime = (strategy == bind::copy) ? sqlite_transient : sqlite_static;
        return ::sqlite3_bind_blob64(stmt, index, static_cast<const void*>(value.data()), value.size_bytes(), lifetime);
    }
};

template <>
struct parameter_binder<zeroblob> {
    auto operator()(::sqlite3_stmt* stmt, int index, zeroblob value) const noexcept -> int {
        return ::sqlite3_bind_zeroblob64(stmt, index, value.size);
    }
};

template <>
struct parameter_binder<std::nullptr_t> {
    auto operator()(::sqlite3_stmt* stmt, int index, std::nullptr_t) const noexcept -> int {
        return ::sqlite3_bind_null(stmt, index);
    }
};

template <typename T>
    requires std::is_same_v<std::remove_const_t<T>, ::sqlite3_value>
struct parameter_binder<T*> {
    auto operator()(::sqlite3_stmt* stmt, int index, const ::sqlite3_value* value) const noexcept -> int {
        return ::sqlite3_bind_value(stmt, index, value);
    }
};

export template <typename T, typename... Opts>
[[nodiscard]] auto bind_parameter_at(statement_handle stmt, int index, T&& value, Opts&&... opts) noexcept
    -> std::expected<void, std::error_code> {
    int result = parameter_binder<std::remove_cvref_t<T>>{}(
        stmt.get(), index, std::forward<T>(value), std::forward<Opts>(opts)...);
    if (result != SQLITE_OK) {
        return std::unexpected(make_sqlite_error_code(result));
    }
    return {};
}

export template <typename T, typename... Opts>
[[nodiscard]] auto bind_named_parameter(statement_handle stmt, const char* name, T&& value, Opts&&... opts) noexcept
    -> std::expected<void, std::error_code> {
    int index = ::sqlite3_bind_parameter_index(stmt.get(), name);
    return bind_parameter_at(stmt, index, std::forward<T>(value), std::forward<Opts>(opts)...);
}

export template <typename T, typename... Opts>
[[nodiscard]] auto bind_parameter(statement_handle stmt, int& index, T&& value, Opts&&... opts) noexcept
    -> std::expected<void, std::error_code> {
    return bind_parameter_at(stmt, index++, std::forward<T>(value), std::forward<Opts>(opts)...);
}

export template <typename... Args>
[[nodiscard]] auto bind_parameters_from(const statement_handle& stmt, int& index, Args&&... args) noexcept
    -> std::expected<void, std::error_code> {
    std::expected<void, std::error_code> result{};
    std::ignore = ((result = bind_parameter(stmt, index, std::forward<Args>(args))) && ...);
    return result;
}

export template <typename... Args>
[[nodiscard]] auto bind_parameters_at(const statement_handle& stmt, int index, Args&&... args) noexcept
    -> std::expected<void, std::error_code> {
    return bind_parameters_from(stmt, index, std::forward<Args>(args)...);
}

export template <typename... Args>
[[nodiscard]] auto bind_parameters(const statement_handle& stmt, Args&&... args) noexcept
    -> std::expected<void, std::error_code> {
    return bind_parameters_at(stmt, 1, std::forward<Args>(args)...);
}

export template <typename Tuple>
[[nodiscard]] auto bind_tuple_from(const statement_handle& stmt, int& index, Tuple&& tuple) noexcept
    -> std::expected<void, std::error_code> {
    return std::apply(
        [stmt, &index](auto&&... args) {
            return bind_parameters_from(stmt, index, std::forward<decltype(args)>(args)...);
        },
        std::forward<Tuple>(tuple));
}

export template <typename Tuple>
[[nodiscard]] auto bind_tuple_at(const statement_handle& stmt, int index, Tuple&& tuple) noexcept
    -> std::expected<void, std::error_code> {
    return bind_tuple_from(stmt, index, std::forward<Tuple>(tuple));
}

export template <typename Tuple>
[[nodiscard]] auto bind_tuple(const statement_handle& stmt, Tuple&& tuple) noexcept
    -> std::expected<void, std::error_code> {
    return bind_tuple_at(stmt, 1, std::forward<Tuple>(tuple));
}

} // namespace sqlixx
