// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#include "sqlite_mock.hpp"

import sqlixx;
import std;

using ::testing::Return;

class SQLiteErrorTest : public ::testing::Test {
protected:
    auto SetUp() -> void override { g_sqlite_mock = &mock_; }

    auto TearDown() -> void override { g_sqlite_mock = nullptr; }

    SQLiteMock mock_;
};

TEST_F(SQLiteErrorTest, CategoryNameIsSqlite3) {
    const auto& cat = sqlixx::sqlite_category();
    EXPECT_STREQ(cat.name(), "sqlite3");
}

TEST_F(SQLiteErrorTest, MessageReturnsStringFromSqlite3) {
    auto errorCode = SQLITE_BUSY;
    const auto* const expectedMessage = "database is locked";

    EXPECT_CALL(mock_, sqlite3_errstr(errorCode)).WillOnce(Return(expectedMessage));

    const auto& cat = sqlixx::sqlite_category();
    auto actualMessage = cat.message(errorCode);

    EXPECT_EQ(actualMessage, expectedMessage);
}

TEST_F(SQLiteErrorTest, MakeErrorCodeCreatesValidErrorCode) {
    auto ec = sqlixx::make_sqlite_error_code(SQLITE_CORRUPT);

    EXPECT_EQ(ec.value(), SQLITE_CORRUPT);
    EXPECT_EQ(&ec.category(), &sqlixx::sqlite_category());
}

TEST_F(SQLiteErrorTest, MakeErrorConditionCreatesValidErrorCondition) {
    auto cond = sqlixx::make_error_condition(sqlixx::sqlite_errc::nomem);

    EXPECT_EQ(cond.value(), SQLITE_NOMEM);
    EXPECT_EQ(&cond.category(), &sqlixx::sqlite_category());
}

TEST_F(SQLiteErrorTest, DefaultErrorConditionMasksExtendedCodes) {
    auto extendedCode = SQLITE_IOERR_READ;

    const auto& cat = sqlixx::sqlite_category();
    auto cond = cat.default_error_condition(extendedCode);

    EXPECT_EQ(cond.value(), SQLITE_IOERR);
    EXPECT_EQ(&cond.category(), &sqlixx::sqlite_category());
}

TEST_F(SQLiteErrorTest, ExtendedErrorCodeMatchesBaseEnumCondition) {
    auto ec = sqlixx::make_sqlite_error_code(SQLITE_IOERR_READ);

    EXPECT_TRUE(ec == sqlixx::sqlite_errc::ioerr);
}

TEST_F(SQLiteErrorTest, IsErrorConditionEnumTraitIsTrue) {
    auto isEnumTraitTrue = std::is_error_condition_enum_v<sqlixx::sqlite_errc>;
    EXPECT_TRUE(isEnumTraitTrue);
}
