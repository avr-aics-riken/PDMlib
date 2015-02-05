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

public:
    virtual ~Read(){}

    //! *dataに、ファイルからデータを読み込む
    //! @param [in]    filename     読み込み対象のファイル名
    //! @param [out]   original_size    データ長（単位はByte)
    //! @param [inout] **data       読み込んだデータを格納する領域へのポインタ
    //! @return -1: エラーによりデータが読めなかった
    //! @return  0: 指定されたファイルが存在しなかった
    //! @return >0: 実際に読み込んだデータサイズ(単位はByte)
    virtual int read(const char* filename, size_t& original_size, char** data) = 0;
};

//! 抽象ファイル読み込みクラス
class ReadFile: public Read
{
public:
    virtual ~ReadFile(){}
    virtual int read(const char* filename, size_t& original_size, char** data) = 0;

protected:
    std::ifstream in;
};

//! バイナリ形式のファイルを読み込む具象クラス
//
class ReadBinaryFile: public ReadFile
{
    friend class ReadFactory;
    ReadBinaryFile(const int& arg_size_of_datatype): need_endian_convert(false),
                                                     size_of_datatype(arg_size_of_datatype){}

public:
    //! @attention 内部でnew char [] するので、*dataに確保済の領域を指定しないこと。
    int read(const char* filename, size_t& original_size, char** data);

private:
    bool isNativeEndian(const int& byte_order_mark);

    template<typename T>
    void convert_endian(T* value)
    {
        char* first = reinterpret_cast<char*>(value);
        char* last  = first+sizeof(T);
        std::reverse(first, last);
    }

    void convert_endian(char* data, const size_t& num_elements, const size_t& size_of_datatype)
    {
        for(int i = 0; i < num_elements; i++)
        {
            char* first = data+size_of_datatype*i;
            char* last  = data+size_of_datatype*(i+1);
            std::reverse(first, last);
        }
    }

    bool   need_endian_convert;
    size_t size_of_datatype;
};

//
// declaration and implimentation of decorator
//
//! 抽象デコレータ
class Decoder: public Read
{
protected:
    Decoder(Read* arg): base(arg),
                        buff(NULL){}

public:
    virtual ~Decoder()
    {
        delete base;
    }

    virtual int read(const char* filename, size_t& original_size, char** data) = 0;

protected:
    Read* base;
    char* buff;
};

//! zip形式による伸張機能を提供する具象デコレータ
class ZipDecoder: public Decoder
{
    friend class ReadFactory;
    explicit ZipDecoder(Read* arg): Decoder(arg){}

public:
    int read(const char* filename, size_t& original_size, char** data);
};

//! fpzip形式による伸張機能を提供する具象デコレータ
class FpzipDecoder: public Decoder
{
    friend class ReadFactory;
    explicit FpzipDecoder(Read* arg, bool is_dp, int arg_vlen): Decoder(arg),
                                                                dp(0),
                                                                vlen(arg_vlen)
    {
        if(is_dp) dp = 1;
    }

public:
    int read(const char* filename, size_t& original_size, char** data);

private:
    int      dp;
    unsigned vlen;
};

//! RLEアルゴリズムによる伸張機能を提供する具象デコレータ
class RLEDecoder: public Decoder
{
    friend class ReadFactory;
    explicit RLEDecoder(Read* arg): Decoder(arg){}

public:
    int read(const char* filename, size_t& original_size, char** data);
};

//! Readクラス用シンプルファクトリ
class ReadFactory
{
public:
    static Read* create(const std::string& decorator, const std::string& type, const int& NumComp);
};
} //end of namespace
#endif
