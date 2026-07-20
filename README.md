# sqlixx: sqlite on C++ steroids

<!--
SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
SPDX-License-Identifier: CC0-1.0
-->

![Library status](https://img.shields.io/badge/Library%20status-under%20development-lightgrey)
[![CI Test](https://github.com/jlmjanssen/sqlixx/actions/workflows/ci-test.yaml/badge.svg)](https://github.com/jlmjanssen/sqlixx/actions/workflows/ci-test.yaml)
[![codecov](https://codecov.io/gh/jlmjanssen/sqlixx/graph/badge.svg?token=CCO7X7SUDG)](https://codecov.io/gh/jlmjanssen/sqlixx)

`sqlixx` is a C++ module library that wraps the SQLite database library.

**Implements**: `sqlixx::connection`, `sqlixx::statement`

**Status**: Experimental

---

## License

`sqlixx` is released under the [MIT License](LICENSE)

---

```C++
auto conn = sqlixx::open_connection("file:data.db?mode=ro&cache=private", sqlixx::open::uri);
auto stmt = sqlixx::prepare_statement(conn, "SELECT u.id, u.name FROM users AS u WHERE u.status = 'active';");
```
