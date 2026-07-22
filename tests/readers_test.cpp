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

class SQLiteReadersTest : public ::testing::Test {
protected:
    auto SetUp() -> void override { g_sqlite_mock = &mock_; }
    auto TearDown() -> void override { g_sqlite_mock = nullptr; }

    SQLiteMock mock_;
    sqlixx::statement_handle stmt_{DUMMY_STMT};
};

TEST_F(SQLiteReadersTest, ReadIntegerTypes) {
    // 3 active columns available
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(3));

    // Standard int (signed 32-bit) -> routed to sqlite3_column_int
    EXPECT_CALL(mock_, sqlite3_column_int(DUMMY_STMT, 0)).WillOnce(Return(42));
    auto res_int = sqlixx::read_column_at<int>(stmt_, 0);
    ASSERT_TRUE(res_int.has_value());
    EXPECT_EQ(*res_int, 42);

    // bool -> smaller than int -> routed to sqlite3_column_int
    EXPECT_CALL(mock_, sqlite3_column_int(DUMMY_STMT, 1)).WillOnce(Return(1));
    auto res_bool = sqlixx::read_column_at<bool>(stmt_, 1);
    ASSERT_TRUE(res_bool.has_value());
    EXPECT_TRUE(*res_bool);

    // uint32_t -> unsigned equal size -> routed to sqlite3_column_int64 to protect high bit
    EXPECT_CALL(mock_, sqlite3_column_int64(DUMMY_STMT, 2)).WillOnce(Return(3000000000LL));
    auto res_uint32 = sqlixx::read_column_at<std::uint32_t>(stmt_, 2);
    ASSERT_TRUE(res_uint32.has_value());
    EXPECT_EQ(*res_uint32, 3000000000U);
}

TEST_F(SQLiteReadersTest, ReadFloatingPointTypes) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(2));

    EXPECT_CALL(mock_, sqlite3_column_double(DUMMY_STMT, 0)).WillOnce(Return(3.14));
    auto res_double = sqlixx::read_column_at<double>(stmt_, 0);
    ASSERT_TRUE(res_double.has_value());
    EXPECT_DOUBLE_EQ(*res_double, 3.14);

    EXPECT_CALL(mock_, sqlite3_column_double(DUMMY_STMT, 1)).WillOnce(Return(2.5));
    auto res_float = sqlixx::read_column_at<float>(stmt_, 1);
    ASSERT_TRUE(res_float.has_value());
    EXPECT_FLOAT_EQ(*res_float, 2.5f);
}

TEST_F(SQLiteReadersTest, ReadStringAndStringView) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(3));
    const char* text_data = "hello";

    // std::string_view
    EXPECT_CALL(mock_, sqlite3_column_text(DUMMY_STMT, 0))
        .WillOnce(Return(reinterpret_cast<const unsigned char*>(text_data)));
    EXPECT_CALL(mock_, sqlite3_column_bytes(DUMMY_STMT, 0)).WillOnce(Return(5));
    auto res_view = sqlixx::read_column_at<std::string_view>(stmt_, 0);
    ASSERT_TRUE(res_view.has_value());
    EXPECT_EQ(*res_view, "hello");

    // std::string
    EXPECT_CALL(mock_, sqlite3_column_text(DUMMY_STMT, 1))
        .WillOnce(Return(reinterpret_cast<const unsigned char*>(text_data)));
    EXPECT_CALL(mock_, sqlite3_column_bytes(DUMMY_STMT, 1)).WillOnce(Return(5));
    auto res_str = sqlixx::read_column_at<std::string>(stmt_, 1);
    ASSERT_TRUE(res_str.has_value());
    EXPECT_EQ(*res_str, "hello");

    // Null text handling (should yield empty string_view)
    EXPECT_CALL(mock_, sqlite3_column_text(DUMMY_STMT, 2)).WillOnce(Return(nullptr));
    auto res_null = sqlixx::read_column_at<std::string_view>(stmt_, 2);
    ASSERT_TRUE(res_null.has_value());
    EXPECT_TRUE(res_null->empty());
}

TEST_F(SQLiteReadersTest, ReadBlobAndSpan) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(2));
    std::array<std::byte, 4> binary_data{std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};

    // std::span<const std::byte>
    EXPECT_CALL(mock_, sqlite3_column_blob(DUMMY_STMT, 0)).WillOnce(Return(binary_data.data()));
    EXPECT_CALL(mock_, sqlite3_column_bytes(DUMMY_STMT, 0)).WillOnce(Return(4));
    auto res_span = sqlixx::read_column_at<std::span<const std::byte>>(stmt_, 0);
    ASSERT_TRUE(res_span.has_value());
    EXPECT_EQ(res_span->size(), 4);
    EXPECT_EQ((*res_span)[0], std::byte{0xDE});

    // std::vector<std::byte>
    EXPECT_CALL(mock_, sqlite3_column_blob(DUMMY_STMT, 1)).WillOnce(Return(binary_data.data()));
    EXPECT_CALL(mock_, sqlite3_column_bytes(DUMMY_STMT, 1)).WillOnce(Return(4));
    auto res_vec = sqlixx::read_column_at<std::vector<std::byte>>(stmt_, 1);
    ASSERT_TRUE(res_vec.has_value());
    EXPECT_EQ(res_vec->size(), 4);
    EXPECT_EQ((*res_vec)[1], std::byte{0xAD});
}

TEST_F(SQLiteReadersTest, ReadSqliteValuePointer) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillOnce(Return(1));
    EXPECT_CALL(mock_, sqlite3_column_value(DUMMY_STMT, 0)).WillOnce(Return(DUMMY_VALUE));

    auto res = sqlixx::read_column_at<const ::sqlite3_value*>(stmt_, 0);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, DUMMY_VALUE);
}

TEST_F(SQLiteReadersTest, ReadColumnIncrementsIndex) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(2));
    EXPECT_CALL(mock_, sqlite3_column_int(DUMMY_STMT, 0)).WillOnce(Return(10));
    EXPECT_CALL(mock_, sqlite3_column_int(DUMMY_STMT, 1)).WillOnce(Return(20));

    int column_index = 0;

    auto val1 = sqlixx::read_column<int>(stmt_, column_index);
    EXPECT_EQ(column_index, 1);
    EXPECT_EQ(*val1, 10);

    auto val2 = sqlixx::read_column<int>(stmt_, column_index);
    EXPECT_EQ(column_index, 2);
    EXPECT_EQ(*val2, 20);
}

TEST_F(SQLiteReadersTest, ReadMultipleColumnsVariadic) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(3));
    EXPECT_CALL(mock_, sqlite3_column_int(DUMMY_STMT, 0)).WillOnce(Return(100));
    EXPECT_CALL(mock_, sqlite3_column_double(DUMMY_STMT, 1)).WillOnce(Return(5.5));

    const char* text_data = "var";
    EXPECT_CALL(mock_, sqlite3_column_text(DUMMY_STMT, 2))
        .WillOnce(Return(reinterpret_cast<const unsigned char*>(text_data)));
    EXPECT_CALL(mock_, sqlite3_column_bytes(DUMMY_STMT, 2)).WillOnce(Return(3));

    int out_int = 0;
    double out_double = 0.0;
    std::string out_str;

    auto res = sqlixx::read_columns(stmt_, out_int, out_double, out_str);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(out_int, 100);
    EXPECT_DOUBLE_EQ(out_double, 5.5);
    EXPECT_EQ(out_str, "var");
}

TEST_F(SQLiteReadersTest, ReadTupleUnpacking) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(2));
    EXPECT_CALL(mock_, sqlite3_column_int(DUMMY_STMT, 0)).WillOnce(Return(1337));
    EXPECT_CALL(mock_, sqlite3_column_double(DUMMY_STMT, 1)).WillOnce(Return(1.1));

    auto res_tuple = sqlixx::read_tuple<int, double>(stmt_);
    ASSERT_TRUE(res_tuple.has_value());

    EXPECT_EQ(std::get<0>(*res_tuple), 1337);
    EXPECT_DOUBLE_EQ(std::get<1>(*res_tuple), 1.1);
}

TEST_F(SQLiteReadersTest, ReadColumnBoundsAndActiveRowErrors) {
    // Scenario 1: No active row available (sqlite3_data_count returns 0)
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillOnce(Return(0));
    auto res_no_row = sqlixx::read_column_at<int>(stmt_, 0);
    ASSERT_FALSE(res_no_row.has_value());
    EXPECT_EQ(res_no_row.error(), sqlixx::errc::no_active_row);

    // Scenario 2: Column index out of bounds
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(2));

    auto res_underflow = sqlixx::read_column_at<int>(stmt_, -1);
    ASSERT_FALSE(res_underflow.has_value());
    EXPECT_EQ(res_underflow.error(), sqlixx::errc::invalid_column_index);

    auto res_overflow = sqlixx::read_column_at<int>(stmt_, 2);
    ASSERT_FALSE(res_overflow.has_value());
    EXPECT_EQ(res_overflow.error(), sqlixx::errc::invalid_column_index);
}

TEST_F(SQLiteReadersTest, ReadStringNullHandlingAndFoldOrder) {
    EXPECT_CALL(mock_, sqlite3_data_count(DUMMY_STMT)).WillRepeatedly(Return(3));

    // Test case 1: Verify std::string gracefully handles database NULL values
    EXPECT_CALL(mock_, sqlite3_column_text(DUMMY_STMT, 0)).WillOnce(Return(nullptr));
    auto res_null_str = sqlixx::read_column_at<std::string>(stmt_, 0);
    ASSERT_TRUE(res_null_str.has_value());
    EXPECT_TRUE(res_null_str->empty());

    // Test case 2: Verify strict left-to-right evaluation order of distinct types in fold expression
    EXPECT_CALL(mock_, sqlite3_column_int(DUMMY_STMT, 1)).WillOnce(Return(50));
    EXPECT_CALL(mock_, sqlite3_column_double(DUMMY_STMT, 2)).WillOnce(Return(4.5));

    int target_int = 0;
    double target_double = 0.0;

    // Column index starts at 1, should fetch int from 1, then double from 2
    auto res_fold = sqlixx::read_columns_at(stmt_, 1, target_int, target_double);
    ASSERT_TRUE(res_fold.has_value());
    EXPECT_EQ(target_int, 50);
    EXPECT_DOUBLE_EQ(target_double, 4.5);
}
