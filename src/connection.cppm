// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;

#include <sqlite3.h>

export module sqlixx:connection;

import std;
import :handles;

namespace sqlixx {

class connection_errmsg_mixin {
public:
    [[nodiscard]] constexpr auto errmsg(this auto self) noexcept -> const char* { return ::sqlite3_errmsg(self.get()); }
};

export using connection = owning_handle<::sqlite3*, ::sqlite3_close_v2, connection_errmsg_mixin>;
export using connection_handle = connection::shallow_handle_type;

} // namespace sqlixx
