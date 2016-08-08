/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "Read.h"

namespace BaseIO
{
Read* ReadFactory::create(const std::string& filename, const std::string& decorator, const std::string& type, const int& NumComp)
{
    int size_of_type = 1;
    if(type == "INT32" || type == "uINT32" || type == "FLOAT")
    {
        size_of_type = 4;
    }else if(type == "INT64" || type == "uINT64" || type == "DOUBLE"){
        size_of_type = 8;
    }

    Read* reader = new ReadBinaryFile(filename, size_of_type);
    const bool need_endian_conversion = !(reader->isNativeEndian());

    //decoratorで指定された内容にしたがってDecoderを追加する
    std::istringstream iss(decorator);
    std::string piece;
    std::vector<std::string> decorator_container;
    while(std::getline(iss, piece, '-'))
    {
        std::transform(piece.begin(), piece.end(), piece.begin(), tolower);
        if(piece == "zip" || piece == "fpzip" || piece == "rle")
        {
            decorator_container.push_back(piece);
        }
    }

    for(std::vector<std::string>::reverse_iterator it = decorator_container.rbegin(); it != decorator_container.rend(); ++it)
    {
        if(*it == "zip")
        {
            reader = new ZipDecoder(reader);
        }else if(*it == "fpzip"){
            //float or double以外の時はfpzip encoderは無視
            if(type == "float")
            {
                reader = new FpzipDecoder(reader, false, NumComp);
            }else if(type == "double"){
                reader = new FpzipDecoder(reader, true, NumComp);
            }
        }else if(*it == "rle"){
            reader = new RLEDecoder(reader);
        }else{
            std::cerr<<"unknown decoder!!"<<std::endl;
        }
    }
    if(need_endian_conversion)
    {
      reader = new ConvertEndian(reader, size_of_type);
    }

    return reader;
}
} //end of namespace
