// SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy <https://github.com/algoritnl>
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

import sqlixx;
import std;

TEST(ErrorTest, CategoryNameIsSqlixx) {
    const auto& cat = sqlixx::sqlixx_category();
    EXPECT_STREQ(cat.name(), "sqlixx");
}

TEST(ErrorTest, MessageMappingWithinBoundaries) {
    const auto& cat = sqlixx::sqlixx_category();

    EXPECT_EQ(cat.message(1), "General error");
    EXPECT_EQ(cat.message(3), "Invalid argument");
}

TEST(ErrorTest, MessageMappingOutsideBoundaries) {
    const auto& cat = sqlixx::sqlixx_category();

    EXPECT_EQ(cat.message(0), "Unknown error");
    EXPECT_EQ(cat.message(-1), "Unknown error");
    EXPECT_EQ(cat.message(99), "Unknown error");
}

TEST(ErrorTest, MakeErrorCodeCreatesValidErrorCode) {
    auto ec = sqlixx::make_error_code(sqlixx::errc::invalid_handle);

    EXPECT_EQ(ec.value(), 2);
    EXPECT_EQ(&ec.category(), &sqlixx::sqlixx_category());
}

TEST(ErrorTest, IsErrorCodeEnumTraitIsTrue) {
    auto isEnumTraitTrue = std::is_error_code_enum_v<sqlixx::errc>;
    EXPECT_TRUE(isEnumTraitTrue);
}

TEST(ErrorTest, ImplicitConversionToErrorCodeMatchesCategory) {
    std::error_code ec = sqlixx::errc::general;

    EXPECT_EQ(ec.value(), 1);
    EXPECT_EQ(ec.message(), "General error");
}

TEST(SqlixxErrorTest, ErrorCodeComparesDirectlyWithEnum) {
    auto ec = sqlixx::make_error_code(sqlixx::errc::invalid_handle);

    EXPECT_TRUE(ec == sqlixx::errc::invalid_handle);
    EXPECT_EQ(ec, sqlixx::errc::invalid_handle);

    EXPECT_FALSE(ec == sqlixx::errc::general);
    EXPECT_NE(ec, sqlixx::errc::general);
}
