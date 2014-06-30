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
bool is_little(void
               );

//! @brief 実行中の処理系がbig-endianかどうかを確認する
//
//! int型の1をファイルに書き込んだ時にLeast Significant Byteが1になっていれば
//! big endianと判定している
bool is_big(void
            );

//! @brief 指定されたファイルが存在するかどうか確認する
bool isFile(const std::string& filename
            );

//! N要素のものをNumProcでブロック分割したときにMyRankが担当する先頭要素のindexを返す
//
//余りは前半Rankが順に担当
//indexは0オリジン
int GetStartIndex(const int& N, const int& NumProc, const int& MyRank);

//! 指定されたディレクトリ以下にあるファイルの一覧を返す
void ListDirectoryContents(std::string dir_name, std::vector<std::string>* filenames);

//! 指定されたディレクトリ以下にあるファイルからタイムステップ部分を抜き出したもののリストを作る
//
//! keywordを含むファイル名から、一番右の'_'と'.'で囲まれた文字をintに変換したリストを作るだけなので
//! この間に数字以外が含まれていると誤動作する
void MakeTimeStepList(std::set<int>* time_steps, const std::string& keyword, const std::string& dir_name = "./", const int& start_time = 0, const int& end_time = INT_MAX);

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
    std::stringstream ss;
    ss<<value;
    return ss.str();
}

//! std::stringをintに変換
int stoi(std::string str
         );

//! std::stringをlongに変換
long stol(std::string str
          );

//! std::stringをlong longに変換
long long stoll(std::string str
                );

//! std::stringをfloatに変換
float stof(std::string str
           );

//! std::stringをdoubleに変換
double stod(std::string str
            );

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
std::string enumType2string(SupportedType enumType
                            );

//! 文字列を PDMlib::SupportedTypeに変換する
SupportedType string2enumType(std::string stringType
                              );

//! PDMlib::StorageOrder を文字列に変換する
std::string enumStorageOrder2string(StorageOrder enumStorageOrder
                                    );

//! 文字列を PDMlib::StorageOrder に変換する
StorageOrder string2enumStorageOrder(std::string stringStorageOrder
                                     );

//! sizeof演算子の代替
size_t GetSize(SupportedType enumType
               );
} //end of namespace
#endif