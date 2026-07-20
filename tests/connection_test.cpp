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

class SQLiteConnectionTest : public ::testing::Test {
protected:
    auto SetUp() -> void override { g_sqlite_mock = &mock_; }

    auto TearDown() -> void override { g_sqlite_mock = nullptr; }

    SQLiteMock mock_;
};

TEST_F(SQLiteConnectionTest, HandleDefaultConstructorAndObservers) {
    sqlixx::connection_handle empty_handle;
    EXPECT_EQ(empty_handle.c_handle(), nullptr);
    EXPECT_FALSE(empty_handle);

    sqlixx::connection_handle active_handle(DUMMY_DB);
    EXPECT_EQ(active_handle.c_handle(), DUMMY_DB);
    EXPECT_TRUE(active_handle);
}

TEST_F(SQLiteConnectionTest, HandleReleaseClearsInternalPointer) {
    sqlixx::connection_handle handle(DUMMY_DB);
    auto* released = handle.release();

    EXPECT_EQ(released, DUMMY_DB);
    EXPECT_EQ(handle.c_handle(), nullptr);
}

TEST_F(SQLiteConnectionTest, ConnectionDestructorCallsCloseIfValid) {
    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));

    sqlixx::connection conn(DUMMY_DB);
}

TEST_F(SQLiteConnectionTest, ConnectionMoveConstructorTransfersOwnership) {
    sqlixx::connection source(DUMMY_DB);

    EXPECT_CALL(mock_, sqlite3_close_v2(_)).Times(0);

    sqlixx::connection destination(std::move(source));

    EXPECT_TRUE(destination);
    EXPECT_EQ(destination.c_handle(), DUMMY_DB);
    EXPECT_FALSE(source);
    EXPECT_EQ(source.c_handle(), nullptr);

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteConnectionTest, ConnectionMoveAssignmentDeallocatesOldAndTakesNew) {
    auto* const DUMMY_DB_OLD = reinterpret_cast<::sqlite3*>(0xAAAAAAA0);
    auto* const DUMMY_DB_NEW = reinterpret_cast<::sqlite3*>(0xBBBBBBB0);

    sqlixx::connection conn_assigned(DUMMY_DB_OLD);
    sqlixx::connection conn_source(DUMMY_DB_NEW);

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB_OLD)).WillOnce(Return(SQLITE_OK));

    conn_assigned = std::move(conn_source);

    EXPECT_EQ(conn_assigned.c_handle(), DUMMY_DB_NEW);
    EXPECT_EQ(conn_source.c_handle(), nullptr);

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB_NEW)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteConnectionTest, ConnectionMoveAssignmentSelfAssignmentDoesNothing) {
    sqlixx::connection conn(DUMMY_DB);

    EXPECT_CALL(mock_, sqlite3_close_v2(_)).Times(0);

    conn = std::move(conn);

    EXPECT_EQ(conn.c_handle(), DUMMY_DB);

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteConnectionTest, ConnectionImplicitlyConvertsToHandle) {
    sqlixx::connection conn(DUMMY_DB);

    sqlixx::connection_handle handle = conn;
    EXPECT_EQ(handle.c_handle(), DUMMY_DB);

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteConnectionTest, OpenConnectionSuccessWithDefaultFlags) {
    const auto* db_name = "test.db";
    auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

    EXPECT_CALL(mock_, sqlite3_open_v2(testing::StrEq(db_name), _, expected_flags, nullptr))
        .WillOnce(DoAll(SetArgPointee<1>(DUMMY_DB), Return(SQLITE_OK)));

    auto result = sqlixx::open_connection(db_name);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->c_handle(), DUMMY_DB);

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteConnectionTest, OpenConnectionSuccessWithCustomFlags) {
    const auto* db_name = "test.db";
    const auto* vfs_name = "unix-dotfile";
    auto expected_flags = SQLITE_OPEN_READONLY | SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_EXRESCODE;

    EXPECT_CALL(mock_, sqlite3_open_v2(testing::StrEq(db_name), _, expected_flags, testing::StrEq(vfs_name)))
        .WillOnce(DoAll(SetArgPointee<1>(DUMMY_DB), Return(SQLITE_OK)));

    auto privatecache = sqlixx::open::dyn_flags(SQLITE_OPEN_PRIVATECACHE);
    auto result = sqlixx::open_connection(db_name, vfs_name, sqlixx::open::readonly, privatecache);

    ASSERT_TRUE(result.has_value());
    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteConnectionTest, OpenConnectionFailureTriggersErrorHandlerAndCleansUpHandle) {
    const auto* db_name = "invalid.db";
    auto error_handler_called = false;

    EXPECT_CALL(mock_, sqlite3_open_v2(testing::StrEq(db_name), _, _, nullptr))
        .WillOnce(DoAll(SetArgPointee<1>(DUMMY_DB), Return(SQLITE_CANTOPEN)));

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));

    auto handler = [&](::sqlite3* db, int rc) noexcept -> void {
        EXPECT_EQ(db, DUMMY_DB);
        EXPECT_EQ(rc, SQLITE_CANTOPEN);
        error_handler_called = true;
    };

    auto result = sqlixx::open_connection(db_name, handler, sqlixx::open::readwrite);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), sqlixx::sqlite_errc::cantopen);
    EXPECT_TRUE(error_handler_called);
}

TEST_F(SQLiteConnectionTest, OpenConnectionFailureWithNullHandleDoesNotCallClose) {
    const auto* db_name = "invalid2.db";

    EXPECT_CALL(mock_, sqlite3_open_v2(testing::StrEq(db_name), _, _, nullptr))
        .WillOnce(DoAll(SetArgPointee<1>(nullptr), Return(SQLITE_NOMEM)));

    EXPECT_CALL(mock_, sqlite3_close_v2(_)).Times(0);

    auto result = sqlixx::open_connection(db_name);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), sqlixx::sqlite_errc::nomem);
}

TEST_F(SQLiteConnectionTest, OpenConnectionWithCustomErrorHandlerVfsAndFlagsSuccess) {
    const auto* db_name = "custom_vfs.db";
    const auto* vfs_name = "unix-none";

    auto expected_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE;

    EXPECT_CALL(mock_, sqlite3_open_v2(testing::StrEq(db_name), _, expected_flags, testing::StrEq(vfs_name)))
        .WillOnce(DoAll(SetArgPointee<1>(DUMMY_DB), Return(SQLITE_OK)));

    auto handler = [&](::sqlite3* db, int rc) noexcept -> void {
        FAIL() << "Error handler should not be called on success.";
    };

    auto result = sqlixx::open_connection(db_name, vfs_name, handler, sqlixx::open::readwrite_create);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->c_handle(), DUMMY_DB);

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));
}

TEST_F(SQLiteConnectionTest, OpenConnectionWithCustomErrorHandlerVfsAndFlagsFailure) {
    const auto* db_name = "custom_vfs_fail.db";
    const auto* vfs_name = "unix-none";
    auto error_handler_called = false;
    auto expected_flags = SQLITE_OPEN_READONLY | SQLITE_OPEN_EXRESCODE;

    EXPECT_CALL(mock_, sqlite3_open_v2(testing::StrEq(db_name), _, expected_flags, testing::StrEq(vfs_name)))
        .WillOnce(DoAll(SetArgPointee<1>(DUMMY_DB), Return(SQLITE_PERM)));

    EXPECT_CALL(mock_, sqlite3_close_v2(DUMMY_DB)).WillOnce(Return(SQLITE_OK));

    auto handler = [&](::sqlite3* db, int rc) noexcept -> void {
        EXPECT_EQ(db, DUMMY_DB);
        EXPECT_EQ(rc, SQLITE_PERM);
        error_handler_called = true;
    };

    auto result = sqlixx::open_connection(db_name, vfs_name, handler, sqlixx::open::readonly);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), sqlixx::sqlite_errc::perm);
    EXPECT_TRUE(error_handler_called);
}
