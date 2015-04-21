/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

#include "zlib.h"
#include "fpzip.h"
#include "BOM.h"
#include "Read.h"

namespace BaseIO
{
int ReadBinaryFile::read(const char* filename, size_t& original_size, char** data)
{
    in.open(filename, std::ios::binary);
    if(in.fail())
    {
        std::cerr<<"file not found! ("<<filename<<")"<<std::endl;
        return 0;
    }

    char size_of_int;
    in.read((char*)&size_of_int,    1);
    char size_of_size_t;
    in.read((char*)&size_of_size_t, 1);
    //TODO size_of_intやsize_of_size_tの値を見て、byte_order_markなどの変数の型を変える

    int byte_order_mark;
    in.read((char*)&byte_order_mark, sizeof(byte_order_mark));
    if(in.fail())
    {
        std::cerr<<"I/O error occurred"<<std::endl;
        return -1;
    }
    need_endian_convert = !isNativeEndian(byte_order_mark);

    in.read((char*)&original_size, sizeof(original_size));
    if(in.fail())
    {
        std::cerr<<"I/O error occurred"<<std::endl;
        return -1;
    }
    if(need_endian_convert)
    {
        convert_endian(&original_size);
    }

    size_t actual_size;
    in.read((char*)&actual_size, sizeof(actual_size));
    if(in.fail())
    {
        std::cerr<<"I/O error occurred"<<std::endl;
        return -1;
    }
    if(need_endian_convert)
    {
        convert_endian(&actual_size);
    }

    *data = new char[actual_size];
    in.read(*data, actual_size);
    if(in.fail())
    {
        std::cerr<<"I/O error occurred"<<std::endl;
        return -1;
    }
    if(need_endian_convert)
    {
        convert_endian(*data, actual_size, size_of_datatype);
    }
    in.close();
    return actual_size;
}

bool ReadBinaryFile::isNativeEndian(const int& byte_order_mark)
{
    return byte_order_mark == BOM;
}

int ZipDecoder::read(const char* filename, size_t& original_size, char** data)
{
    size_t src_size = base->read(filename, original_size, data);
    if(src_size <= 0)
    {
        return src_size;
    }

    buff = new char[original_size];
    size_t dest_size = original_size;

    if(uncompress((Bytef*)buff, &dest_size, (Bytef*)*data, src_size) != Z_OK)
    {
        std::cerr<<"Zip decod failed"<<std::endl;
        delete buff;
        return -1;
    }else{
        delete (char*)*data;
        *data = buff;
        buff  = NULL;
    }
    return dest_size;
}

int FpzipDecoder::read(const char* filename, size_t& original_size, char** data)
{
    size_t src_size = base->read(filename, original_size, data);
    if(src_size <= 0)
    {
        return src_size;
    }

    buff = new char[original_size];

    int    num_elements = dp ? original_size/8 : original_size/4;

    size_t dest_size    = fpzip_memory_read(*data, buff, NULL, dp, num_elements, 1, 1, vlen);
    if(dest_size <= 0)
    {
        std::cerr<<"Fpzip decod failed"<<std::endl;
        return -1;
    }else{
        delete (char*)*data;
        *data = buff;
        buff  = NULL;
    }
    // fpzip_memory_readの戻り値は実際に読んだ（伸長前の）データサイズ
    return original_size;
}

int RLEDecoder::read(const char* filename, size_t& original_size, char** data)
{
    size_t src_size = base->read(filename, original_size, data);
    if(src_size <= 0)
    {
        return src_size;
    }

    buff = new char[original_size];
    size_t dest_size = original_size;

    if(uncompress((Bytef*)buff, &dest_size, (Bytef*)*data, src_size) != Z_OK)
    {
        std::cerr<<"Zip decod failed"<<std::endl;
        delete buff;
        return -1;
    }else{
        delete (char*)*data;
        *data = buff;
        buff  = NULL;
    }
    return dest_size;
}
} //end of namespace