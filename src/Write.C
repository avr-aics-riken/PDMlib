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
#include "Write.h"

namespace BaseIO
{
int WriteTextFile::write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data)
{
    WriteFile::out.open(filename);
    for(int i = 0; i < actual_size/size_of_type; i++)
    {
        if(type == "int")
        {
            WriteFile::out<<reinterpret_cast<int*>(data)[i];
        }else if(type == "long"){
            WriteFile::out<<reinterpret_cast<long*>(data)[i];
        }else if(type == "unsigned int"){
            WriteFile::out<<reinterpret_cast<unsigned int*>(data)[i];
        }else if(type == "unsigned long"){
            WriteFile::out<<reinterpret_cast<unsigned long*>(data)[i];
        }else if(type == "float"){
            WriteFile::out<<reinterpret_cast<float*>(data)[i];
        }else if(type == "double"){
            WriteFile::out<<reinterpret_cast<double*>(data)[i];
        }else{
            WriteFile::out<<data[i];
        }
        WriteFile::out<<delimiter;
    }
    WriteFile::out<<std::endl;
    WriteFile::out.close();
    return actual_size;
}

int WriteBinaryFile::write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data)
{
    const int byte_order_mark = BOM;
    WriteFile::out.open(filename, std::ios::binary);
    char      size_of_int = sizeof(int);
    WriteFile::out.write(&size_of_int, 1);
    char      size_of_size_t = sizeof(size_t);
    WriteFile::out.write(&size_of_size_t, 1);

    WriteFile::out.write((char*)&byte_order_mark, sizeof(int));
    WriteFile::out.write((char*)&original_size, sizeof(size_t));
    WriteFile::out.write((char*)&actual_size, sizeof(size_t));
    WriteFile::out.write((char*)data, actual_size);
    WriteFile::out.close();
    return actual_size;
}

int ZipEncoder::write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data)
{
    size_t output_size = compressBound(actual_size);
    buff = new char[output_size];
    if(compress((Bytef*)buff, &output_size, (Bytef*)data, (uLong)actual_size) != Z_OK)
    {
        std::cerr<<"Zip Encode failed. this container is not compressed!"<<std::endl;
        return Encoder::base->write(filename, original_size, actual_size, data);
    }
    return Encoder::base->write(filename, original_size, output_size, buff);
}

int FpzipEncoder::write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data)
{
    buff = new char[(size_t)(actual_size*1.2)];
    size_t length      = dp ? original_size/8 : original_size/4;
    size_t output_size = fpzip_memory_write(buff, actual_size, data, NULL, dp, length, 1, 1, vlen);
    return Encoder::base->write(filename, original_size, output_size, buff);
}

int RLEEncoder::write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data)
{
    size_t   output_size = compressBound(actual_size);
    buff = new char[output_size];

    z_stream z;
    z.zalloc    = Z_NULL;
    z.zfree     = Z_NULL;
    z.opaque    = Z_NULL;
    z.avail_in  = actual_size;
    z.next_in   = (Bytef*)data;
    z.avail_out = output_size;
    z.next_out  = (Bytef*)buff;

    // windowBits(第4引数）=15, memLevel(第5引数)=8はDeflateInitを呼んだ時の設定値と同じ
    if(deflateInit2(&z, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15, 8, Z_RLE) != Z_OK)
    {
        std::cerr<<"zlib initialize failed. this container is not compressed!"<<std::endl;
        goto error;
    }
    if(deflate(&z, Z_FINISH) != Z_STREAM_END)
    {
        std::cerr<<"RLE encode failed. this container is not compressed!"<<std::endl;
        goto error;
    }

    output_size = z.total_out;
    deflateEnd(&z);

    return Encoder::base->write(filename, original_size, output_size, buff);

error:
    return Encoder::base->write(filename, original_size, actual_size, data);
}
} //end of namespace