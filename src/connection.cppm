// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;

#include <sqlite3.h>

export module sqlixx:connection;

import std;
import :handles;

namespace sqlixx {

export using connection = owning_handle<::sqlite3*, ::sqlite3_close_v2>;
export using connection_handle = connection::shallow_handle_type;

} // namespace sqlixx
