// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#include "sqlite_mock.hpp"

import sqlixx;
import std;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

inline const auto DUMMY_DB = reinterpret_cast<::sqlite3*>(0xDEADC0DE);
inline const auto DUMMY_STMT = reinterpret_cast<::sqlite3_stmt*>(0x00FEE16D);

class SQLiteStatementTest : public ::testing::Test {
protected:
    auto SetUp() -> void override { g_sqlite_mock = &mock_; }

    auto TearDown() -> void override { g_sqlite_mock = nullptr; }

    SQLiteMock mock_;
};

TEST_F(SQLiteStatementTest, HandleDefaultConstructorAndObservers) {
    sqlixx::statement_handle empty_handle;
    EXPECT_EQ(empty_handle.get(), nullptr);
    EXPECT_FALSE(empty_handle);

    sqlixx::statement_handle active_handle(DUMMY_STMT);
    EXPECT_EQ(active_handle.get(), DUMMY_STMT);
    EXPECT_TRUE(active_handle);
}

TEST_F(SQLiteStatementTest, HandleReleaseClearsInternalPointer) {
    sqlixx::statement_handle handle(DUMMY_STMT);
    auto* const released = handle.release();

    EXPECT_EQ(released, DUMMY_STMT);
    EXPECT_EQ(handle.get(), nullptr);
}

TEST_F(SQLiteStatementTest, StatementDestructorCallsFinalizeIfValid) {
    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));

    {
        sqlixx::statement stmt(DUMMY_STMT);
    }
}

TEST_F(SQLiteStatementTest, StatementMoveConstructorTransfersOwnership) {
    sqlixx::statement source(DUMMY_STMT);

    EXPECT_CALL(mock_, sqlite3_finalize(_)).Times(0);

    sqlixx::statement destination(std::move(source));

    EXPECT_EQ(destination.get(), DUMMY_STMT);
    EXPECT_EQ(source.get(), nullptr);

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, StatementMoveAssignmentDeallocatesOldAndTakesNew) {
    auto* const DUMMY_STMT_OLD = reinterpret_cast<::sqlite3_stmt*>(0xAAAAAAA0);
    auto* const DUMMY_STMT_NEW = reinterpret_cast<::sqlite3_stmt*>(0xBBBBBBB0);

    sqlixx::statement stmt_assigned(DUMMY_STMT_OLD);
    sqlixx::statement stmt_source(DUMMY_STMT_NEW);

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT_OLD)).WillOnce(Return(SQLITE_OK));

    stmt_assigned = std::move(stmt_source);

    EXPECT_EQ(stmt_assigned.get(), DUMMY_STMT_NEW);
    EXPECT_EQ(stmt_source.get(), nullptr);

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT_NEW)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, StatementMoveAssignmentSelfAssignmentDoesNothing) {
    sqlixx::statement stmt(DUMMY_STMT);

    EXPECT_CALL(mock_, sqlite3_finalize(_)).Times(0);

    stmt = std::move(stmt);

    EXPECT_EQ(stmt.get(), DUMMY_STMT);

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, StatementImplicitlyConvertsToHandle) {
    sqlixx::statement stmt(DUMMY_STMT);

    sqlixx::statement_handle handle = stmt;
    EXPECT_EQ(handle.get(), DUMMY_STMT);

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, PrepareStatementCStringSuccessWithDefaultFlags) {
    sqlixx::connection_handle db(DUMMY_DB);
    const auto* const sql = "SELECT * FROM users;";

    EXPECT_CALL(mock_, sqlite3_prepare_v3(DUMMY_DB, testing::StrEq(sql), -1, 0, _, _))
        .WillOnce(DoAll(SetArgPointee<4>(DUMMY_STMT), SetArgPointee<5>(sql + std::strlen(sql)), Return(SQLITE_OK)));

    auto result = sqlixx::prepare_statement(db, sql);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stmt.get(), DUMMY_STMT);
    EXPECT_TRUE(result->tail.empty());

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, PrepareStatementWithDynamicFlags) {
    sqlixx::connection_handle db(DUMMY_DB);
    const auto* const sql = "SELECT 1;";

    auto dynamic_persistent = sqlixx::prep::dyn_flags(SQLITE_PREPARE_PERSISTENT);

    EXPECT_CALL(mock_, sqlite3_prepare_v3(DUMMY_DB, testing::StrEq(sql), -1, SQLITE_PREPARE_PERSISTENT, _, _))
        .WillOnce(DoAll(SetArgPointee<4>(DUMMY_STMT), SetArgPointee<5>(sql + std::strlen(sql)), Return(SQLITE_OK)));

    auto result = sqlixx::prepare_statement(db, sql, dynamic_persistent);

    ASSERT_TRUE(result.has_value());
    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, PrepareStatementStringViewSuccessWithTailAndCustomFlags) {
    using namespace std::string_view_literals;
    sqlixx::connection_handle db(DUMMY_DB);
    auto sql = "SELECT 1; SELECT 2;"sv;

    auto expected_len = static_cast<int>(sql.size());
    auto expected_flags = SQLITE_PREPARE_PERSISTENT | SQLITE_PREPARE_NORMALIZE;

    const auto* const expected_tail_ptr = sql.data() + 9;

    EXPECT_CALL(mock_, sqlite3_prepare_v3(DUMMY_DB, sql.data(), expected_len, expected_flags, _, _))
        .WillOnce(DoAll(SetArgPointee<4>(DUMMY_STMT), SetArgPointee<5>(expected_tail_ptr), Return(SQLITE_OK)));

    auto result = sqlixx::prepare_statement(db, sql, sqlixx::prep::persistent, sqlixx::prep::normalize);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stmt.get(), DUMMY_STMT);

    EXPECT_EQ(result->tail, " SELECT 2;"sv);

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, PrepareStatementStringViewWithNoRemainingTail) {
    using namespace std::string_view_literals;
    sqlixx::connection_handle db(DUMMY_DB);
    auto sql = "SELECT 1;"sv;

    auto expected_len = static_cast<int>(sql.size());
    const auto* const expected_tail_ptr = sql.data() + sql.size();

    EXPECT_CALL(mock_, sqlite3_prepare_v3(DUMMY_DB, sql.data(), expected_len, 0, _, _))
        .WillOnce(DoAll(SetArgPointee<4>(DUMMY_STMT), SetArgPointee<5>(expected_tail_ptr), Return(SQLITE_OK)));

    auto result = sqlixx::prepare_statement(db, sql);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stmt.get(), DUMMY_STMT);

    EXPECT_TRUE(result->tail.empty());

    EXPECT_CALL(mock_, sqlite3_finalize(DUMMY_STMT)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteStatementTest, PrepareStatementFailureReturnsErrorNoCleanUpNeeded) {
    sqlixx::connection_handle db(DUMMY_DB);
    const auto* const sql = "SELECT INVALID SYNTAX;";

    EXPECT_CALL(mock_, sqlite3_prepare_v3(DUMMY_DB, testing::StrEq(sql), -1, 0, _, _))
        .WillOnce(DoAll(SetArgPointee<4>(nullptr), Return(SQLITE_ERROR)));

    EXPECT_CALL(mock_, sqlite3_finalize(_)).Times(0);

    auto result = sqlixx::prepare_statement(db, sql);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), sqlixx::sqlite_errc::error);
}
