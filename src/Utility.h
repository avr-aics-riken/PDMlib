/*
###################################################################################
#
# PDMlib - Particle Data Management library
#
# Copyright (c) 2014-2017 Advanced Institute for Computational Science(AICS), RIKEN.
# All rights reserved.
#
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University.
# All rights reserved.
#
###################################################################################
*/

#ifndef PDMLIB_UTIL_H
#define PDMLIB_UTIL_H
#include <cstdlib>
#include <string>
#include <set>
#include <climits>
#include <sstream>
#include "PDMlib.h"

namespace PDMlib
{
//! @brief 実行中の処理系がlittle-endianかどうかを確認する
//
//! int型の1をファイルに書き込んだ時にMost Significant Byteが1になっていれば
//! little  endianと判定している
bool is_little(void);

//! @brief 実行中の処理系がbig-endianかどうかを確認する
//
//! int型の1をファイルに書き込んだ時にLeast Significant Byteが1になっていれば
//! big endianと判定している
bool is_big(void);

//! @brief 指定されたファイルが存在するかどうか確認する
bool isFile(const std::string& filename);

//! N要素のものをNumProcでブロック分割したときにMyRankが担当する先頭要素のindexを返す
//
//余りは前半Rankが順に1要素づつ追加で担当するものとする
//indexは0オリジン
int GetStartIndex(const int& N, const int& NumProc, const int& MyRank);

//! 指定されたディレクトリ以下にあるファイルの一覧を返す
//
//! glob(3)に対するラッパー
void ListDirectoryContents(const std::string& dir_name, std::vector<std::string>* filenames, const std::string& wild_card = "*");

//! @brief 引数で渡された文字列が全て数字かどうかを判定
bool is_all_digit(const std::string& str);

//! @brief 引数で渡された文字列からタイムステップと思われる数字を抽出して返す
//
//! 実際には文字列のsuffix(最後のピリオド以降)を除いた文字列から
//! 最後のアンダーバー以降の文字列を抽出して返している
//! オプション引数のrank_stepがfalseの時は末尾から2つ目のアンダーバーから
//! 最後のアンダーバーまでの文字列を抽出して返している
//
//! @param [in] filename 抽出元の文字列
//! @rt -1      filename中に'.'および'_'が1つも無かった
//! @rt -2      タイムステップと想定される文字列に数値以外の文字が含まれていた
int get_time_step(const std::string& filename, const bool& is_rank_step);

//! 引数で渡された文字列から領域番号と思われる数字を抽出して返す
//
//! 引数のrank_stepフラグを否定してget_time_stepを呼ぶだけ
//
//! @param [in] filename 抽出元の文字列
//! @rt -1      filename中に'_'が2つ以上存在していなかった
//! @rt -2      領域番号と想定される文字列に数値以外の文字が含まれていた
int get_region_number(const std::string& filename, const bool& is_rank_step);


//
// C++11非対応な環境向けの独自実装ルーチン
//
//! @brief 引数で与えられた値をstd::stringに変換して返す
//
// GCCの実装だと型ごとにvsnprintfを呼び出してstringのコンストラクタに渡しているけど
// 全部用意しても使わない可能性が高いのでtemplate+stringstreamを使って手抜き実装
template<typename T>
std::string to_string(T value)
{
#if __cplusplus >= 201103L
    return std::to_string(value);
#else
    std::stringstream ss;
    ss<<value;
    return ss.str();
#endif
}

//! std::stringをintに変換
int stoi_wrapper(std::string str);

//! std::stringをdoubleに変換
double stod_wrapper(std::string str);

//
// UnitElem 構造体のソート用プレディケータ
//
//!名前の辞書順にソートする
struct UnitElemSorter
{
    bool operator()(const UnitElem& left, const UnitElem& right) const
    {
        return left.Name < right.Name;
    }
};

//! ContainerInfo 構造体のソート用プレディケータ
//
//!名前の辞書順にソートする
struct ContainerInfoSorter
{
    bool operator()(const ContainerInfo& left, const ContainerInfo& right) const
    {
        return left.Name < right.Name;
    }
};

//
// PDMlibが定義するenumに関する変換ツール
//
//! PDMlib::SupportedType を文字列に変換する
std::string enumType2string(SupportedType enumType);

//! 文字列を PDMlib::SupportedTypeに変換する
SupportedType string2enumType(std::string stringType);

//! PDMlib::StorageOrder を文字列に変換する
std::string enumStorageOrder2string(StorageOrder enumStorageOrder);

//! 文字列を PDMlib::StorageOrder に変換する
StorageOrder string2enumStorageOrder(std::string stringStorageOrder);

//! sizeof演算子の代替
size_t GetSize(SupportedType enumType);


/// 指定されたパスが存在しなければディレクトリを作成する
bool mkdir_if_not_exist(const std::string& path);

/// 再帰的に指定されたパスのディレクトリを作成する
bool RecursiveMkdir(const std::string& path);
} //end of namespace
#endif
