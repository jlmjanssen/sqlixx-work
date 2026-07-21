// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#include "sqlite_mock.hpp"

import sqlixx;
import std;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;

inline const auto DUMMY_STMT = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);
inline const auto DUMMY_VALUE = reinterpret_cast<::sqlite3_value*>(0x00BEEF00);

class SQLiteBindersTest : public ::testing::Test {
protected:
    auto SetUp() -> void override { g_sqlite_mock = &mock_; }
    auto TearDown() -> void override { g_sqlite_mock = nullptr; }

    SQLiteMock mock_;
    sqlixx::statement_handle stmt_{DUMMY_STMT};
};

TEST_F(SQLiteBindersTest, BindIntegerTypes) {
    // Test small types (int format now routed to int64)
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 1, 42)).WillOnce(Return(SQLITE_OK));
    auto res_int = sqlixx::bind_parameter_at(stmt_, 1, 42);
    EXPECT_TRUE(res_int.has_value());

    // Test boolean types (treated as integrals, now routed to int64)
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 2, 1)).WillOnce(Return(SQLITE_OK));
    auto res_bool = sqlixx::bind_parameter_at(stmt_, 2, true);
    EXPECT_TRUE(res_bool.has_value());

    // Test large types (int64 format)
    std::int64_t large_val = 9000000000000LL;
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 3, large_val)).WillOnce(Return(SQLITE_OK));
    auto res_int64 = sqlixx::bind_parameter_at(stmt_, 3, large_val);
    EXPECT_TRUE(res_int64.has_value());
}

TEST_F(SQLiteBindersTest, BindFloatingPointTypes) {
    EXPECT_CALL(mock_, sqlite3_bind_double(DUMMY_STMT, 1, 3.14)).WillOnce(Return(SQLITE_OK));
    auto res_double = sqlixx::bind_parameter_at(stmt_, 1, 3.14);
    EXPECT_TRUE(res_double.has_value());

    EXPECT_CALL(mock_, sqlite3_bind_double(DUMMY_STMT, 2, 2.5f)).WillOnce(Return(SQLITE_OK));
    auto res_float = sqlixx::bind_parameter_at(stmt_, 2, 2.5f);
    EXPECT_TRUE(res_float.has_value());
}

TEST_F(SQLiteBindersTest, BindStringTypesWithStrategies) {
    using namespace std::string_view_literals;

    // C-String (Default strategy: copy -> SQLITE_TRANSIENT)
    const char* c_str = "hello";
    EXPECT_CALL(mock_, sqlite3_bind_text(DUMMY_STMT, 1, c_str, -1, reinterpret_cast<void (*)(void*)>(-1)))
        .WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 1, c_str).has_value());

    // String literal array (size - 1, SQLITE_TRANSIENT)
    EXPECT_CALL(mock_,
                sqlite3_bind_text(DUMMY_STMT, 2, ::testing::StrEq("test"), 4, reinterpret_cast<void (*)(void*)>(-1)))
        .WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 2, "test").has_value());

    // std::string_view with shallow bind (SQLITE_STATIC)
    auto view = "static_text"sv;
    EXPECT_CALL(mock_, sqlite3_bind_text(DUMMY_STMT, 3, view.data(), 11, reinterpret_cast<void (*)(void*)>(0)))
        .WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 3, view, sqlixx::bind::shallow).has_value());
}

TEST_F(SQLiteBindersTest, BindSpanBlobTypes) {
    std::array<std::uint8_t, 4> data{0xDE, 0xAD, 0xBE, 0xEF};
    std::span<std::uint8_t> blob_span(data);

    // Default strategy: copy -> SQLITE_TRANSIENT
    EXPECT_CALL(mock_, sqlite3_bind_blob64(DUMMY_STMT, 1, blob_span.data(), 4, SQLITE_TRANSIENT))
        .WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 1, blob_span).has_value());

    // Strategy: shallow -> SQLITE_STATIC
    EXPECT_CALL(mock_, sqlite3_bind_blob64(DUMMY_STMT, 2, blob_span.data(), 4, SQLITE_STATIC))
        .WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 2, blob_span, sqlixx::bind::shallow).has_value());
}

TEST_F(SQLiteBindersTest, BindSpecialTypes) {
    // Zeroblob
    EXPECT_CALL(mock_, sqlite3_bind_zeroblob64(DUMMY_STMT, 1, 1024U)).WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 1, sqlixx::zeroblob{.size = 1024U}).has_value());

    // Nullptr
    EXPECT_CALL(mock_, sqlite3_bind_null(DUMMY_STMT, 2)).WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 2, nullptr).has_value());

    // Sqlite3_value pointer
    EXPECT_CALL(mock_, sqlite3_bind_value(DUMMY_STMT, 3, DUMMY_VALUE)).WillOnce(Return(SQLITE_OK));
    EXPECT_TRUE(sqlixx::bind_parameter_at(stmt_, 3, DUMMY_VALUE).has_value());
}

TEST_F(SQLiteBindersTest, BindNamedParameter) {
    const char* param_name = ":user_id";
    EXPECT_CALL(mock_, sqlite3_bind_parameter_index(DUMMY_STMT, testing::StrEq(param_name))).WillOnce(Return(5));
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 5, 1337)).WillOnce(Return(SQLITE_OK));

    auto res = sqlixx::bind_named_parameter(stmt_, param_name, 1337);
    EXPECT_TRUE(res.has_value());
}

TEST_F(SQLiteBindersTest, BindParameterIncrementsIndex) {
    int index = 1;
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 1, 10)).WillOnce(Return(SQLITE_OK));
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 2, 20)).WillOnce(Return(SQLITE_OK));

    EXPECT_TRUE(sqlixx::bind_parameter(stmt_, index, 10).has_value());
    EXPECT_EQ(index, 2);

    EXPECT_TRUE(sqlixx::bind_parameter(stmt_, index, 20).has_value());
    EXPECT_EQ(index, 3);
}

TEST_F(SQLiteBindersTest, BindMultipleParameters) {
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 1, 100)).WillOnce(Return(SQLITE_OK));
    EXPECT_CALL(mock_, sqlite3_bind_double(DUMMY_STMT, 2, 5.5)).WillOnce(Return(SQLITE_OK));
    EXPECT_CALL(mock_, sqlite3_bind_null(DUMMY_STMT, 3)).WillOnce(Return(SQLITE_OK));

    auto res = sqlixx::bind_parameters(stmt_, 100, 5.5, nullptr);
    EXPECT_TRUE(res.has_value());
}

TEST_F(SQLiteBindersTest, BindTupleUnpacking) {
    auto data_tuple = std::make_tuple(42, 3.14, "text");

    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 1, 42)).WillOnce(Return(SQLITE_OK));
    EXPECT_CALL(mock_, sqlite3_bind_double(DUMMY_STMT, 2, 3.14)).WillOnce(Return(SQLITE_OK));

    // Match exact array size (4) and the TRANSIENT destructor pointer (-1)
    EXPECT_CALL(mock_,
                sqlite3_bind_text(DUMMY_STMT, 3, ::testing::StrEq("text"), -1, reinterpret_cast<void (*)(void*)>(-1)))
        .WillOnce(Return(SQLITE_OK));

    auto res = sqlixx::bind_tuple(stmt_, data_tuple);
    EXPECT_TRUE(res.has_value());
}

TEST_F(SQLiteBindersTest, BindParameterFailurePropagatesError) {
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 1, 99)).WillOnce(Return(SQLITE_TOOBIG));

    auto res = sqlixx::bind_parameter_at(stmt_, 1, 99);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), sqlixx::sqlite_errc::toobig); // Assuming make_sqlite_error_code maps this correctly
}

TEST_F(SQLiteBindersTest, BindParametersShortCircuitOnFailure) {
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 1, 10)).WillOnce(Return(SQLITE_OK));
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 2, 20)).WillOnce(Return(SQLITE_MISMATCH));

    // The third binding must never be called due to short-circuiting in the fold expression
    EXPECT_CALL(mock_, sqlite3_bind_int64(DUMMY_STMT, 3, 30)).Times(0);

    auto res = sqlixx::bind_parameters(stmt_, 10, 20, 30);
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), sqlixx::sqlite_errc::mismatch);
}
