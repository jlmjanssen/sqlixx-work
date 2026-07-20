// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;
#include <sqlite3.h>

export module sqlixx:connection;
import std;
import :sqlite_error;
import :handles;

namespace sqlixx {

export using connection_handle = shallow_handle<::sqlite3*>;
export using connection = owning_handle<::sqlite3*>;

template <>
struct handle_deleter<::sqlite3*> {
    auto operator()(::sqlite3* handle) const noexcept -> void { ::sqlite3_close_v2(handle); }
};

namespace open {
struct flag_t {};

template <int flags>
struct flags_t : flag_t {
    [[nodiscard]] constexpr auto get() const noexcept -> int { return flags; }
};

export struct dyn_flags : flag_t {
    explicit constexpr dyn_flags(int flags) noexcept : flags_(flags) {}
    [[nodiscard]] constexpr auto get() const noexcept -> int { return flags_; }

private:
    int flags_;
};

using readonly_t = flags_t<SQLITE_OPEN_READONLY>;
using readwrite_t = flags_t<SQLITE_OPEN_READWRITE>;
using readwrite_create_t = flags_t<SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE>;
using memory_t = flags_t<SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY>;
using uri_t = flags_t<SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI>;

export constexpr readonly_t readonly;
export constexpr readwrite_t readwrite;
export constexpr readwrite_create_t readwrite_create;
export constexpr memory_t memory;
export constexpr uri_t uri;

template <typename Flag>
concept is_flag = std::is_base_of_v<flag_t, std::decay_t<Flag>> && requires(std::decay_t<Flag> flags) {
    { flags.get() } noexcept -> std::convertible_to<int>;
};

template <typename... Flags>
concept all_flags = (is_flag<Flags> && ...);

template <typename... Flags>
    requires all_flags<Flags...>
constexpr auto make_flags(Flags... flags) noexcept -> int {
    if constexpr (sizeof...(Flags) == 0) {
        return SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;
    } else {
        return (SQLITE_OPEN_EXRESCODE | ... | flags.get());
    }
}
} // namespace open

constexpr auto default_error_handler = [](::sqlite3*, int) noexcept -> void {};

template <typename Handler>
concept error_handler = requires(Handler handler, ::sqlite3* handle, int result) {
    { handler(handle, result) } -> std::same_as<void>;
};

template <typename ErrorHandler = decltype(default_error_handler)>
[[nodiscard]] constexpr auto open_connection_impl(const char* filename,
                                                  int flags,
                                                  const char* vfs,
                                                  ErrorHandler&& on_error = decltype(default_error_handler){}) noexcept
    -> std::expected<connection, std::error_code> {
    ::sqlite3* handle = nullptr;
    int result = ::sqlite3_open_v2(filename, &handle, flags, vfs);

    if (result != SQLITE_OK) {
        std::forward<ErrorHandler>(on_error)(handle, result);
        if (handle != nullptr) {
            ::sqlite3_close_v2(handle);
        }
        return std::unexpected(make_sqlite_error_code(result));
    }

    return connection(handle);
}

export template <open::is_flag... Flags>
[[nodiscard]] constexpr auto open_connection(const char* filename, Flags... flags) noexcept
    -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), nullptr);
}

export template <open::is_flag... Flags>
[[nodiscard]] constexpr auto open_connection(const char* filename, const char* vfs, Flags... flags) noexcept
    -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), vfs);
}

export template <typename ErrorHandler, open::is_flag... Flags>
    requires error_handler<ErrorHandler>
[[nodiscard]] constexpr auto open_connection(const char* filename, ErrorHandler&& on_error, Flags... flags) noexcept
    -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), nullptr, std::forward<ErrorHandler>(on_error));
}

export template <typename ErrorHandler, open::is_flag... Flags>
    requires error_handler<ErrorHandler>
[[nodiscard]] constexpr auto
open_connection(const char* filename, const char* vfs, ErrorHandler&& on_error, Flags... flags) noexcept
    -> std::expected<connection, std::error_code> {
    return open_connection_impl(filename, open::make_flags(flags...), vfs, std::forward<ErrorHandler>(on_error));
}

} // namespace sqlixx
