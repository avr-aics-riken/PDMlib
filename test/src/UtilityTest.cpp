/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include <vector>
#include <iostream>
#include <sstream>
#include "gtest/gtest.h"
#include "Utility.h"


// int GetStartIndex(const int& N, const int& NumProc, const int& MyRank);
/*
TEST(GetStartIndexTest, illegal)
{
}
*/

//bool is_all_digit(std::string& str);
TEST(IsAllDigitTest, ok)
{
  EXPECT_TRUE(PDMlib::is_all_digit("0123456"));
  EXPECT_TRUE(PDMlib::is_all_digit("927401"));
  EXPECT_TRUE(PDMlib::is_all_digit("740981"));
}
TEST(IsAllDigitTest, with_alpha)
{
  EXPECT_FALSE(PDMlib::is_all_digit("0a29jl5"));
  EXPECT_FALSE(PDMlib::is_all_digit("92l41"));
  EXPECT_FALSE(PDMlib::is_all_digit("7409n1"));
}
TEST(IsAllDigitTest, negative_number)
{
  EXPECT_FALSE(PDMlib::is_all_digit("-0123456"));
  EXPECT_FALSE(PDMlib::is_all_digit("-927401"));
  EXPECT_FALSE(PDMlib::is_all_digit("-740981"));
}
TEST(IsAllDigitTest, float)
{
  EXPECT_FALSE(PDMlib::is_all_digit("012.3456"));
  EXPECT_FALSE(PDMlib::is_all_digit("92.7401e+8"));
  EXPECT_FALSE(PDMlib::is_all_digit("7409.81D0"));
}

TEST(GetTimeStepTest, ok)
{
    EXPECT_EQ(0,     PDMlib::get_time_step("foo_bar_baz_000000.c"));
    EXPECT_EQ(1,     PDMlib::get_time_step("foo_bar_baz_000001.c"));
    EXPECT_EQ(3,     PDMlib::get_time_step("foo_3.c"));
    EXPECT_EQ(28710, PDMlib::get_time_step("foo_28710.c"));
}
TEST(GetTimeStepTest, wrong)
{
    EXPECT_EQ(-1, PDMlib::get_time_step("28710.c"));
    EXPECT_EQ(-2, PDMlib::get_time_step("foo_bar_baz_000000.c.hoge"));
    EXPECT_EQ(-2, PDMlib::get_time_step("foo_bar_baz_000001.c.hoge"));
    EXPECT_EQ(-1, PDMlib::get_time_step("foo_28710"));
    EXPECT_EQ(-2, PDMlib::get_time_step("foo_bar.baz"));
    EXPECT_EQ(-2, PDMlib::get_time_step("foo_2124_bar.baz"));
    EXPECT_EQ(-2, PDMlib::get_time_step("foo_bar_baz_-00100.c"));
}
TEST(GetRegionNumberTest, ok)
{
    EXPECT_EQ(0,     PDMlib::get_region_number("foo_bar_baz_000000_3215.c"));
    EXPECT_EQ(1,     PDMlib::get_region_number("foo_bar_baz_000001_3215.c"));
    EXPECT_EQ(0,     PDMlib::get_region_number("foo_bar_baz_000000_3215.c.hoge"));
    EXPECT_EQ(1,     PDMlib::get_region_number("foo_bar_baz_000001_3215.c.hoge"));
    EXPECT_EQ(3,     PDMlib::get_region_number("foo_3_3215.c"));
    EXPECT_EQ(28710, PDMlib::get_region_number("foo_28710_3215.c"));
    EXPECT_EQ(0,     PDMlib::get_region_number("foo_bar_baz_000000_aweqah.c"));
    EXPECT_EQ(1,     PDMlib::get_region_number("foo_bar_baz_000001_aweqah.c"));
    EXPECT_EQ(0,     PDMlib::get_region_number("foo_bar_baz_000000_aweqah.c.hoge"));
    EXPECT_EQ(1,     PDMlib::get_region_number("foo_bar_baz_000001_aweqah.c.hoge"));
    EXPECT_EQ(3,     PDMlib::get_region_number("foo_3_aweqah.c"));
    EXPECT_EQ(28710, PDMlib::get_region_number("foo_28710_aweqah.c"));
    EXPECT_EQ(0,     PDMlib::get_region_number("foo_bar_baz_000000_aweqah"));
    EXPECT_EQ(1,     PDMlib::get_region_number("foo_bar_baz_000001_aweqah"));
    EXPECT_EQ(3,     PDMlib::get_region_number("foo_3_aweqah"));
    EXPECT_EQ(28710, PDMlib::get_region_number("foo_28710_aweqah"));
    EXPECT_EQ(28710, PDMlib::get_region_number("foo_28710_hoge"));
}
TEST(GetRegionNumberTest, wrong)
{
    EXPECT_EQ(-1, PDMlib::get_region_number("28710"));
    EXPECT_EQ(-1, PDMlib::get_region_number("28710_c"));
    EXPECT_EQ(-2, PDMlib::get_region_number("foo_bar_baz"));
    EXPECT_EQ(-2, PDMlib::get_region_number("425_bar_210"));
    EXPECT_EQ(-2, PDMlib::get_region_number("foo_bar_baz_-00100_3215.c"));
    EXPECT_EQ(-2, PDMlib::get_region_number("foo_bar_baz_-00100_aweqah.c"));
    EXPECT_EQ(-2, PDMlib::get_region_number("foo_bar_baz_-00100_aweqah"));
}
 
 //! @brief 引数で渡された文字列からタイムステップと思われる数字を抽出して返す
 //
 //! 実際には文字列内の一番右にある'.'から一番右にある'_'までの部分を数値に変換して返している。
 //
 //! @param [in] filename 抽出元の文字列
 //! @rt -1      filename中に'.'および'_'が1つも無かった
 //int GetTimeStepTest(std::string& filename);
 
 //! 引数で渡された文字列から領域番号と思われる数字を抽出して返す
 //
 //! 実際には文字列内の一番右にある'_'と右から二番目にある'_'までの部分を数値に変換して返している。
 //
 //! @param [in] filename 抽出元の文字列
 //! @rt -1      filename中に'_'が2つ以上存在していなかった
 //int GetRegionNumberTest(std::string& filename);
 

