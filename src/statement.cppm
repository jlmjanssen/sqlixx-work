// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

module;

#include <sqlite3.h>

export module sqlixx:statement;

import std;
import :handles;

namespace sqlixx {

export using statement = owning_handle<::sqlite3_stmt*, ::sqlite3_finalize>;
export using statement_handle = statement::shallow_handle_type;

} // namespace sqlixx
