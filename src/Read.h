/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#ifndef PDMLIB_READ_H
#define PDMLIB_READ_H
#include <iostream>
#include <fstream>
#include <algorithm>
#include "BOM.h"

namespace BaseIO
{
//forward declaration
class ReadFactory;

//! ファイル読み込み基底クラス
class Read
{
    //non-copyable
    Read(const Read&);
    Read& operator=(const Read&);

protected:
    Read(){}
    Read(const std::string& arg_filename, const size_t& arg_size_of_datatype):filename(arg_filename), size_of_datatype(arg_size_of_datatype) {}

public:
    virtual ~Read(){}

    //! *dataに、ファイルからデータを読み込む
    //! @param [out]   original_size      データ長（単位はByte)
    //! @param [inout] **data             読み込んだデータを格納する領域へのポインタ
    //! @return -1: エラーによりデータが読めなかった
    //! @return  0: 指定されたファイルが存在しなかった
    //! @return >0: 実際に読み込んだデータサイズ(単位はByte)
    virtual int read(size_t& original_size, char** data) = 0;

    //! ファイルに記録されたBOMが現在の処理系と一致するかどうか判定する
    bool isNativeEndian();

protected:
    size_t size_of_datatype;
    std::string filename;

    template<typename T>
    void convert_endian(T* value)
    {
        char* first = reinterpret_cast<char*>(value);
        char* last  = first+sizeof(T);
        std::reverse(first, last);
    }
    friend class ReadFactory;
};

//! バイナリ形式のファイルを読み込む具象クラス
//
class ReadBinaryFile: public Read
{
    friend class ReadFactory;
    ReadBinaryFile(const std::string& filename, const int& size_of_datatype): Read(filename, size_of_datatype) {}

public:
    //! @attention 内部でnew char [] するので、*dataに確保済の領域を指定しないこと。
    int read(size_t& original_size, char** data);

    //! 引数で渡されたBOMが現在の処理系のものと一致するかどうかを判定する
    bool isNativeEndian(const int& byte_order_mark);
};

//
// declaration and implimentation of decorator
//
//! 抽象デコレータ
class Decoder: public Read
{
protected:
    Decoder(Read* arg) : base(arg), buff(NULL){}

public:
    virtual ~Decoder()
    {
        delete base;
    }


protected:
    Read* base;
    char* buff;
};

//! zip形式による伸張機能を提供する具象デコレータ
class ZipDecoder: public Decoder
{
    friend class ReadFactory;
    explicit ZipDecoder(Read* arg) : Decoder(arg){}

public:
    int read(size_t& original_size, char** data);
};

//! fpzip形式による伸張機能を提供する具象デコレータ
class FpzipDecoder: public Decoder
{
    friend class ReadFactory;
    explicit FpzipDecoder(Read* arg, bool is_dp, int arg_vlen) : Decoder(arg),
        dp(0),
        vlen(arg_vlen)
    {
        if(is_dp)dp = 1;
    }

public:
    int read(size_t& original_size, char** data);

private:
    int dp;
    unsigned vlen;
};

//! RLEアルゴリズムによる伸張機能を提供する具象デコレータ
class RLEDecoder: public Decoder
{
    friend class ReadFactory;
    explicit RLEDecoder(Read* arg) : Decoder(arg) {}

public:
    int read(size_t& original_size, char** data);
};

//! エンディアン変換機能を提供する具象デコレータ
class ConvertEndian: public Decoder
{
    friend class ReadFactory;
    explicit ConvertEndian(Read* arg, const int& arg_size_of_datatype) : Decoder(arg), size_of_datatype(arg_size_of_datatype) {}
    int size_of_datatype;
public:
    int read(size_t& original_size, char** data);
};

//! Readクラス用シンプルファクトリ
class ReadFactory
{
public:
    static Read* create(const std::string& filename, const std::string& decorator, const std::string& type, const int& NumComp);
};
} //end of namespace
#endif
