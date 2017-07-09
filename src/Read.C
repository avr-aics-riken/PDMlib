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

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

#include <zlib.h>
#include <fpzip.h>
#include "BOM.h"
#include "Read.h"

namespace BaseIO
{
  bool Read::isNativeEndian()
  {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::binary);
    if(in.fail())
    {
        std::cerr<<"file not found! ("<<filename<<")"<<std::endl;
        return 0;
    }

    char size_of_int;
    in.read((char*)&size_of_int,    1);
    char size_of_size_t;
    in.read((char*)&size_of_size_t, 1);

    int byte_order_mark;
    in.read((char*)&byte_order_mark, sizeof(byte_order_mark));
    if(in.fail())
    {
        std::cerr<<"I/O error occurred"<<std::endl;
        return -1;
    }
    in.close();
    return byte_order_mark == BOM;
  }

int ReadBinaryFile::read(size_t& original_size, char** data)
{
    std::ifstream in;
    in.open(filename.c_str(), std::ios::binary);
    if(in.fail())
    {
        std::cerr<<"file not found! ("<<filename<<")"<<std::endl;
        return 0;
    }

    char size_of_int;
    in.read((char*)&size_of_int,    1);
    char size_of_size_t;
    in.read((char*)&size_of_size_t, 1);

    int byte_order_mark;
    in.read((char*)&byte_order_mark, sizeof(byte_order_mark));
    if(in.fail())
    {
        std::cerr<<"I/O error occurred"<<std::endl;
        return -1;
    }
    const bool need_endian_conversion = !isNativeEndian(byte_order_mark);

    in.read((char*)&original_size, sizeof(original_size));
    if(in.fail())
    {
        std::cerr<<"I/O error occurred"<<std::endl;
        return -1;
    }
    if(need_endian_conversion)
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
    if(need_endian_conversion)
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
    in.close();
    return actual_size;
}

bool ReadBinaryFile::isNativeEndian(const int& byte_order_mark)
{
    return byte_order_mark == BOM;
}

int ZipDecoder::read(size_t& original_size, char** data)
{
    size_t src_size = base->read(original_size, data);
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

int FpzipDecoder::read(size_t& original_size, char** data)
{
    size_t src_size = base->read(original_size, data);
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
    // fpzip_memory_readの戻り値は実際に読んだ（伸長前の）データサイズなのでdest_sizeは返さない
    return original_size;
}

int RLEDecoder::read(size_t& original_size, char** data)
{
    size_t src_size = base->read(original_size, data);
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
int ConvertEndian::read(size_t& original_size, char** data)
{
    size_t size_in_byte = base->read(size_in_byte, data);
    size_t num_elements=size_in_byte/size_of_datatype;
    for(int i = 0; i < num_elements; i++)
    {
      char* first = *data+size_of_datatype*i;
      char* last  = *data+size_of_datatype*(i+1);
      std::reverse(first, last);
    }
    return size_in_byte;
}
} //end of namespace
